/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2026 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QString>
#include <QDebug>
#include <limits>

#include "FileTransfer/FileTransfer.h"
#include "ProcessDataStream/StructuredField.h"

/*
 * IND$FILE Processing
 *
 * IND$FILE has a number of commands in DFT mode:
 *
 * Open Req       - Open a transfer
 * Insert Req     - Receive a block of data - could be a message or a data (this preceeds Insert Data)
 * Set Cursor Req - Unsupported; ignored in Q3270
 * Get Req        - Send a block of data
 * Close Req      - Close the file transfer
 *
 * Most of these need to be acknowledged:
 *
 * Open Ack   - Acknowledge the Open Req
 * Normal Ack - Acknowledge the Insert Data
 * Get Ack    - Acknowledge the Get Req
 * Close Ack  - Acknowledge the Close Req
 *
 * Errors (including the non-error, EOF) are returned using the Error Header:
 *
 * Error Open - returned when the Open Req fails
 * Error Get  - returned when the Get Req fails
 * Error EOF  - returned when the Get Req reaches the end of file
 *
 * IND$FILE also returns a number of messages as Insert Data after an Open Req:
 *
 * https://bitsavers.trailing-edge.com/pdf/ibm/pc/3270/3270_PC/SC23-0103-2_3270_PC_Control_Program_Users_Guide_and_Reference_Jul84.pdf
 * Appendix E
 *
 * ZTRANS03 - Transfer completed successfully
 * ZTRANS04 - File transfer complete, with records segmented
 * ZTRANS13 - Error writing file to host: file transfer canceled
 * ZTRANS14 - Error reading file from host: file transfer canceled
 * ZTRANS15 - Required host storage unavailable: file transfer canceled
 * ZTRANS16 - Incorrect request code: file transfer canceled
 * ZTRANS17 - Missing or incorrect TSO data set name: file transfer canceled
 * ZTRANS17 - Missing or incorrect CMS file identifier: file transfer canceled
 * ZTRANS18 - Incorrect options specified: file transfer canceled
 * ZTRANS19 - Error reading or writing to disk: file transfer canceled
 * ZTRANS28 - Invalid option XXXXXXXX: file transfer canceled
 * ZTRANS29 - Invalid option XXXXXXXX with RECEIVE: file transfer canceled
 * ZTRANS30 - Invalid option XXXXXXXX with APPEND: file transfer canceled
 * ZTRANS31 - Invalid option XXXXXXXX without SPACE: file transfer canceled
 * ZTRANS32 - Invalid option XXXXXXXX with PDS: file transfer canceled
 * ZTRANS33 - Only one of TRACKS, CYLINDERS, AVBLOCK allowed: file transfer canceled
 * ZTRANS34 - CMS file not found: file transfer canceled
 * ZTRANS35 - CMS disk is Read-Only: file transfer canceled
 * ZTRANS36 - CMS disk is not accessed: file transfer canceled
 * ZTRANS37 - CMS disk is full: file transfer canceled
 * ZTRANS40 - Member already exists: file transfer canceled
 * ZTRANS99 - Host program error code XX XXXXXXXX: file transfer canceled
 *
 * Of these, only ZTRANS03 is a success message; with ZTRANS04 being a qualified success message
 * (the file was segmented). All others are errors. Many of the errors would be prevented by GUI
 * validation of the file transfer options before sending the Open Req.
 *
 * For IND$FILE in CUT mode, the screen is formatted with blanks, a skip attribute in 1919 and
 * an enterable field in
 */

/**
 * @brief   FileTransfer::FileTransfer - initialise the file transfer object
 *
 * @details The file transfer object is used to handle the IND$FILE structured fields. It is signalled
 *          when a file transfer is started, and processes the requests for reading or writing local files.
 */
FileTransfer::FileTransfer()
{
    setState(FTState::NONE);
}

/**
 * @brief   FileTransfer::setProperties - set the properties for the file transfer
 * @param   filename  - the local file name to read/write
 * @param   direction - the direction of the file transfer (send or receive)
 * @param   ascii     - true for ASCII transfer, false for binary
 * @param   crlf      - true to convert CRLF to LF on receive, false otherwise
 *
 * @details This function is called before starting a file transfer to set the properties of
 *          the file transfer.
 */
void FileTransfer::setProperties(QString filename, Q3270::TransferDirection direction, bool ascii, bool crlf, bool dftEnabled, quint16 dftBlockSize)
{
    this->filename = filename;
    this->direction = direction;
    this->ascii = ascii;
    this->crlf = crlf;
    this->dftEnabled = dftEnabled;
    this->dftBlockSize = dftBlockSize;

    ftMode = FTMode::NONE;
    setState(FTState::EXPECTED);
}

/**
 * @brief   FileTransfer::open - process the Open Req structured field
 * @param   buffer - the buffer containing the Open Req data
 * @param   reply  - the reply buffer to send back to the host
 * @param   wsfLen - the length of the structured field
 *
 * @details This function processes the Open Req structured field. It checks the packet type and opens
 *          the file for reading or writing, depending on the direction of the transfer.
 */
void FileTransfer::open(QByteArray::Iterator &buffer, QByteArray &reply, int wsfLen)
{
    if (ftState == FTState::ABORTING) {
        finishDFTCancellation(reply, IBM3270_FT_ERROR_OPEN);
        return;
    }

    QString packetType;

    qDebug() << "ftOpen - File Transfer Open command received";

    ftMode = FTMode::DFT;

    switch (wsfLen) {
        // Receive
        case 0x23:
            packetType = QString::fromLatin1(reinterpret_cast<const char *>(buffer + 23), 7);
            qDebug() << "FTopen() - Packet type:" << packetType;
            break;
        // Send
        case 0x29: {
            const quint8 *p = reinterpret_cast<const quint8 *>(buffer);
            packetType = QString::fromLatin1(reinterpret_cast<const char *>(p + 29), 7);
            qDebug() << "FTopen() - Packet type:" << packetType;
            break;
        }
        default:
            ftAbort("Unknnown open", IBM3270_FT_ERROR_OPEN, reply);
            return;
    }

    // Set to zero in case of errors
    blocksTransferred = 0;

    // Messages are sent as OPEN REQ and the INSERT DATA request that follows is for the message not
    // the data. The packet type is FT:MSG for messages and FT:DATA for data.
    if (packetType.slice(0, 6) == "FT:MSG")
        ftMsg = true;
    else {
        bytesTransferred = 0;
        emit ftUpdateCounts();
        ftMsg = false;
        switch (direction) {
            case Q3270::TransferDirection::Receive:
                incomingFile.setFileName(filename);
                if (!incomingFile.open(QIODevice::WriteOnly)) {
                    qDebug() << "ftOpen - File Transfer Open failed to open file for Write:" << filename;
                    ftAbort(incomingFile.errorString(), IBM3270_FT_ERROR_OPEN, reply);
                    return;
                }
                break;
            case Q3270::TransferDirection::Send:
                outgoingFile.setFileName(filename);
                if (!outgoingFile.open(QIODevice::ReadOnly)) {
                    qDebug() << "ftOpen - File Transfer Open failed to open file for Read:" << filename;
                    ftAbort(outgoingFile.errorString(), IBM3270_FT_ERROR_OPEN, reply);
                    return;
                }
                pendingDFTLF = false;
                break;
            default:
                qDebug() << "ftOpen - File Transfer Open unknown direction";
        }
        ftState = FTState::EXPECTED;
    }

    // Open sets record count to 1
    blocksTransferred = 1;

    reply.append(IBM3270_AID_SF);

    reply.append(StructuredField(IBM3270_WSF_TRANSFER).
                 addHalfWord(IBM3270_FT_OPEN_ACK).
                 toSF());
}

/**
 * @brief   FileTransfer::insertReq - process the Insert Req structured field
 *
 * @details This function is called when the Insert Req structured field is received. It does not do anything
 *          in Q3270, but is included as the protocol requires it. The Insert Req is followed by the Insert Data
 *           structured field, which contains the actual data
 */
void FileTransfer::insertReq()
{
    if (ftState == FTState::ABORTING)
        return;

    ftMode = FTMode::DFT;
    ftState = FTState::IN_PROGRESS;
}

/**
 * @brief   FileTransfer::insertData - process the Insert Data structured field
 * @param   buffer - the incoming data buffer
 * @param   reply  - the reply buffer to send back to the host
 *
 * @details This function processes the Insert Data structured field. It extracts the data from the buffer,
 *          writes it to the local file, and acknowledges the data. If the Insert Data is a message, it emits
 *          a signal with the message.
 */
void FileTransfer::insertData(QByteArray::Iterator &buffer, QByteArray &reply)
{
    qDebug() << "ftInsertData - File Transfer Insert Data command received";

    if (ftState == FTState::ABORTING) {
        finishDFTCancellation(reply, IBM3270_FT_ERROR_INSERT);
        return;
    }

    ftMode = FTMode::DFT;
    ftState = FTState::IN_PROGRESS;

    // Skip past C0 80 61 static bytes
    // These, according to x3270, are compression indicator (C080) and start of data flags (61)
    buffer += 3;

    // TODO: make buffer its own class
    //quint16 pktLen = nextBufferHalfWord();

    quint16 hi = static_cast<quint8>(*buffer++);
    quint16 lo = static_cast<quint8>(*buffer++);
    quint16 pktLen = ((hi << 8) | lo) - 5; // Subtract 5 for the 5 bytes of header (C0 80 61 + pktLen hi + pktLen lo)

    qDebug() << "ftInsertData - pktLen = " << pktLen << " bytes";

    if (ftMsg) {
        QString msg = QString::fromLatin1(reinterpret_cast<const char *>(buffer), pktLen);
        int dollar = msg.indexOf('$');
        if (dollar > 0)
            msg = msg.left(dollar);
        qDebug() << msg << bytesTransferred << "bytes received";
        emit ftUpdateMessage(msg);
        ftDataAck(reply);
        setState(FTState::NONE);
        emit ftSuccess();
        return;
    }

    if (pktLen > 0) {
        // If this is an ASCII transfer with CRLF, remove the CR and EOF characters
        if (ascii && crlf) {
            ftBlock.clear();
            for (int i = 0; i < pktLen; i++) {
                uint c = buffer[i];
                if (c == 0x1A || c == '\r') {
                    // Skip EOF character and carriage return
                    continue;
                }
                ftBlock.append(c);
            }
            incomingFile.write(ftBlock, ftBlock.length());
        } else {
            /* Write data out */
            incomingFile.write(reinterpret_cast<const char *>(buffer), pktLen);
        }
        if (incomingFile.error() != QFile::NoError) {
            ftAbort(incomingFile.errorString(), IBM3270_FT_ERROR_INSERT, reply);
        }
        bytesTransferred += pktLen;
        emit ftUpdateCounts();
    }

    ftDataAck(reply);
}

/**
 * @brief   FileTransfer::setCursorReq - process the Set Cursor Req structured field
 *
 * @details This function is called when the Set Cursor Req structured field is received. It does not do anything
 *          in Q3270, but is included as the protocol requires it.
 */
void FileTransfer::setCursorReq()
{
    ftMode = FTMode::DFT;
}

/**
 * @brief   FileTransfer::getData - process the Get Req structured field
 * @param   reply - the reply buffer to send back to the host
 *
 * @details This function processes the Get Req structured field. It reads a block of data from the local file,
 *          and sends it back to the host. If the end of file is reached, it sends an EOF error.
 *
 *          If CRLF conversion is enabled, it converts LF to CRLF before sending the data.
 */
void FileTransfer::getData(QByteArray &reply)
{
    if (ftState == FTState::ABORTING) {
        finishDFTCancellation(reply, IBM3270_FT_ERROR_GET);
        return;
    }

    ftMode = FTMode::DFT;
    ftState = FTState::IN_PROGRESS;

    constexpr qsizetype structuredFieldOverhead = 16;
    constexpr qsizetype dftBlockHeadroom = 27;
    constexpr qsizetype maxStructuredFieldPayload = std::numeric_limits<quint16>::max() - structuredFieldOverhead;
    
    if (dftBlockSize > 0 && dftBlockSize <= dftBlockHeadroom) {
        ftAbort("DFT block size is too small for a data packet", IBM3270_FT_ERROR_GET, reply);
        return;
    }

    const qsizetype blockSize = dftBlockSize == 0
                                    ? maxStructuredFieldPayload
                                    : qMin(dftBlockSize - dftBlockHeadroom,
                                      maxStructuredFieldPayload);

    const bool convertLineEndings = ascii && crlf;
    QByteArray data;
    data.reserve(blockSize);

    while (data.size() < blockSize) {
        if (pendingDFTLF) {
            data.append('\n');
            pendingDFTLF = false;
            continue;
        }

        const QByteArray nextByte = outgoingFile.peek(1);
        if (nextByte.isEmpty())
            break;

        const char sourceByte = nextByte.at(0);
        if (convertLineEndings && sourceByte == '\n') {
            const qsizetype remainingCapacity = blockSize - data.size();
            if (remainingCapacity == 1) {
                data.append('\r');
                pendingDFTLF = true;
            } else if (remainingCapacity >= 2) {
                data.append("\r\n", 2);
            } else {
                break;
            }
        } else {
            data.append(sourceByte);
        }

        char consumedByte;
        if (!outgoingFile.getChar(&consumedByte))
            break;
    }

    if (outgoingFile.error() != QFile::NoError) {
        ftAbort(outgoingFile.errorString(), IBM3270_FT_ERROR_GET, reply);
        return;
    }

    if (data.isEmpty() && outgoingFile.atEnd() && !pendingDFTLF) {
        reply.append(IBM3270_AID_SF);
        reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                         .addHalfWord(IBM3270_FT_ERROR_GET)
                         .addHalfWord(IBM3270_FT_ERROR_HDR)
                         .addHalfWord(IBM3270_FT_ERROR_EOF)
                         .toSF());
        qDebug() << "ftGetData - File Transfer Get Data end of file reached";
        return;
    }

    ftBlock.clear();
    ftBlock.append(IBM3270_AID_SF);

    StructuredField sendData = StructuredField(IBM3270_WSF_TRANSFER)
                                   .addHalfWord(IBM3270_FT_GET_ACK)
                                   .addHalfWord(IBM3270_FT_RECNUM_HDR)
                                   .addWord(blocksTransferred)
                                   .addHalfWord(IBM3270_FT_NOTCOMP)
                                   .addByte(IBM3270_FT_DATA_START)
                                   .addHalfWord(data.length() + 5); // Add 5 for the 5 bytes of header


    for (int i = 0; i < data.length(); i++) {
       sendData.addByte(data[i]);
    }

    ftBlock.append(sendData.toSF());

    reply.append(ftBlock);

    bytesTransferred += data.length();
    blocksTransferred++;

    emit ftUpdateCounts();
}

/**
 * @brief   FileTransfer::close - process the Close Req structured field
 * @param   reply - the reply buffer to send back to the host
 *
 * @details This function processes the Close Req structured field. It closes the local file, and sends a Close Ack
 *          back to the host. If the transfer was a receive, it commits the file to disk.
 */
void FileTransfer::close(QByteArray &reply)
{
    qDebug() << "ftClose - File Transfer Close";

    if (ftState == FTState::ABORTING) {
        finishDFTCancellation(reply, IBM3270_FT_ERROR_CLOSE);
        return;
    }

    ftMode = FTMode::DFT;

    if (direction == Q3270::TransferDirection::Send) {
        outgoingFile.close();

    } else if (!incomingFile.commit()) {
        qDebug() << "ftClose - File Transfer Close failed to commit file:" << filename;
        ftAbort(incomingFile.errorString(), IBM3270_FT_ERROR_CLOSE, reply);
        return;
    }

    reply.append(IBM3270_AID_SF);

    reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                     .addHalfWord(IBM3270_FT_CLOSE_ACK)
                     .toSF());

    setState(FTState::NONE);

    qDebug() << "ftClose - File Transfer complete - total bytes received" << bytesTransferred;
}

/**
 * @brief   FileTransfer::ftDataAck - acknowledge the Insert Data structured field
 * @param   reply - the reply buffer to send back to the host
 *
 * @details This function sends a Normal Ack for the Insert Data structured field. It increments the block count.
 */
void FileTransfer::ftDataAck(QByteArray &reply)
{
    reply.append(IBM3270_AID_SF);

    reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                     .addHalfWord(IBM3270_FT_NORMAL_ACK)
                     .addHalfWord(IBM3270_FT_RECNUM_HDR)
                     .addWord(blocksTransferred)
                     .toSF());

    blocksTransferred++;
}

/**
 * @brief   FileTransfer::ftAbort - abort the file transfer
 * @param   msg       - the error message to send back to the host
 * @param   abortType - the type of error to send back to the host
 * @param   reply     - the reply buffer to send back to the host
 *
 * @details This function sends an Error Header for the file transfer. It emits a signal to indicate that the transfer was aborted.
 */
void FileTransfer::ftAbort(QString msg, quint16 abortType, QByteArray &reply)
{
    qDebug() << "ftAbort - File Transfer aborted:" << msg;

    reply.append(IBM3270_AID_SF);

    reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                  .addHalfWord(abortType)
                  .addHalfWord(IBM3270_FT_ERROR_CMD)
                  .toSF());

    emit ftCancel();
}

void FileTransfer::readModified(QByteArray &reply)
{
    if (ftBlock.size())
    {
        reply.append(ftBlock);
    }
}

/**
 * @brief   FileTransfer::reset - reset the file transfer state
 *
 * @details This function resets the file transfer state. It is called when a new file transfer
 *          is expected and when one is complete.
 */
void FileTransfer::setState(FileTransfer::FTState state)
{
    ftState = state;

    if (ftState == FTState::NONE) {
        ftMode = FTMode::NONE;
    }

    if (ftState == FTState::EXPECTED) {
        blocksTransferred = 0;
        bytesTransferred = 0;
        emit ftUpdateCounts();
        emit ftStarted();
    }
}

/**
 * @brief   FileTransfer::getBytes - get the number of bytes transferred
 *
 * @details This function returns the number of bytes transferred. It is used to update the UI
 *          with the progress of the transfer.
 */
quint64 FileTransfer::getBytes()
{
    return bytesTransferred;
}

/**
 * @brief   FileTransfer::getBlocks - get the number of blocks transferred
 *
 * @details This function returns the number of blocks transferred. It is used to update the UI
 *          with the progress of the transfer.
 */
quint64 FileTransfer::getBlocks()
{
    return blocksTransferred;
}

/**
 * @brief   FileTransfer::isExpected - check if a file transfer is expected
 *
 * @details This function returns true if a file transfer is expected, false otherwise. It is used to
 *          determine if the incoming structured fields should be processed as part of a file transfer.
 */
FileTransfer::FTState FileTransfer::getState()
{
    return ftState;
}

/**
 * @brief   FileTransfer::checkCUT - check for CUT mode transfer
 * @param   screen - the display screen object
 * @param   reply  - the reply buffer to send back to the host
 *
 * @details This function checks if a CUT mode transfer is in progress and passes 
 *          control to the appopriate handler.
 */
void FileTransfer::checkCUT(DisplayScreen *screen, QByteArray &reply)
{
    qDebug() << "FileTransfer::checkCUT - Checking for CUT mode transfer - expected " << (ftState == FileTransfer::FTState::EXPECTED);

    if (ftState == FileTransfer::FTState::NONE)
        return;

    if (ftMode == FTMode::DFT)
        return;

    if (screen->isFieldStart(IBM3270_FT_CUT_FIELD) && screen->isAskip(IBM3270_FT_CUT_FIELD))
    {
        quint8 frameType = screen->getChar(IBM3270_FT_FRAMETYPE);

        qDebug() << QString("CUT frame type = %1").arg(frameType, 2, 16);

        ftMode = FTMode::CUT;

        switch (frameType)
        {
            case IBM3270_FT_CUT_DATA:
                cutData(screen, reply);
                break;
            case IBM3270_FT_CUT_DATAREQ:
                cutDataReq(screen, reply);
                break;
            case IBM3270_FT_CUT_CONTROL:
                cutControlCode(screen, reply);
                break;
            case IBM3270_FT_CUT_RETRANS:
                cutRetransmit(reply);
                break;
            default:
                qDebug() << "Unknown CUT frame type";
                reply.append(IBM3270_AID_F2);
                break;
        }
    }
}

/** 
 * @brief   FileTransfer::cutData - process the CUT mode data frame
 * @param   screen - the display screen object
 * @param   reply  - the reply buffer to send back to the host
 * 
 * @details This function processes the CUT mode data frame. It extracts the data
 *          from the screen, writes it to the local file, and acknowledges the data. When the end of file
 *          is reached, it sends an EOF error (which is really just an acknowledgement of the end of the transfer).
 */
void FileTransfer::cutData(DisplayScreen *screen, QByteArray &reply)
{

    if (ftState == FileTransfer::FTState::ABORTING)
    {
        cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABX);
        qDebug() << "FileTransfer::cutData - File Transfer is aborting, ignoring data";
        return;
    }   

    setState(FileTransfer::FTState::IN_PROGRESS);

    // Open file if it's the first time through
    if (!incomingFile.isOpen())
    {
        blocksTransferred = 0;
        bytesTransferred = 0;
        emit ftUpdateCounts();
        incomingFile.setFileName(filename);
        if (!incomingFile.open(QIODevice::WriteOnly)) {
            qDebug() << "cutData - File Transfer Open failed to open file for Write:" << filename;
            emit ftUpdateMessage(incomingFile.errorString());
            cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABX);
            return;
        }
    }

//    screen->dumpDisplay();

    // Clear the block buffer for the incoming data
    ftBlock.clear();

    int dataLen = fromBase64(screen->getChar(IBM3270_FT_CUT_DATA_LEN)) << 6 | fromBase64(screen->getChar(IBM3270_FT_CUT_DATA_LEN + 1));

    qDebug() << "FileTransfer::cutData - Incoming data - length " << dataLen;

    // Detect EOF as 5C A9 in the first two bytes of the data.
    // This is a special case that indicates the end of the file transfer.
    if (dataLen == 2)
    {
        if (screen->getChar(IBM3270_FT_CUT_DATA_STRT) == 0x5C && screen->getChar(IBM3270_FT_CUT_DATA_STRT + 1) == 0xA9)
        {
            qDebug() << "FileTransfer::cutData - Incoming data - EOF";
            reply.append(IBM3270_AID_ENTER);
            return;
        }
    }

    int loopCount = 1;

    // Loop count is used when the quadrant needs to be reset. If the quadrant is reset,
    // the quadrant needs to be re-evaluated for the current character.
    for (int dp = 0; dp < dataLen; dp += loopCount)
    {
        loopCount = 1;

        quint8 thisChar = screen->getChar(IBM3270_FT_CUT_DATA_STRT + dp);

        if (!quadrant)
        {
            if (!(quadrant = findQuadrantByCode(thisChar)))
            {
                qDebug() << "Failed to find quadrant for" << QString("%1").arg(thisChar, 2, 16);
                cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABX);
            }
            else
                qDebug() << "Quadrant for " << QString("%1").arg(thisChar, 2, 16) << "is" << quadrant;
            continue;
        }

        quint8 asc = e2a[thisChar];
        const char* pos = strchr(alphas, asc);

        if (pos == nullptr)
        {
            qDebug() << "Character not mapped; resetting quadrant" << QString("%1").arg(thisChar, 2, 16);
            quadrant = nullptr;
            loopCount = 0;
            continue;
        }

        int index = pos - alphas;
        if (quadrant->map[index] == 0x00 && quadrant != &quadrants[2] && thisChar != 0xC1)
        {
            qDebug() << "Returned character was NULL but this quadrant isn't valid for that. Resetting";
            loopCount = 0;
        }

        quint8 c = quadrant->map[index];

        // Handle ASCII and CRLF conversion. If the character is a CR or EOF, skip it.
        if (ascii && crlf)
        {
            if (c == 0x1A || c == '\r')
            {
                // Skip EOF character and carriage return
                continue;
            }
        }

        ftBlock.append(c);
    }

    incomingFile.write(ftBlock, ftBlock.length());

    if (incomingFile.error() != QFile::NoError)
    {
        emit ftUpdateMessage(incomingFile.errorString());
        cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABF);
    }

    bytesTransferred += ftBlock.size();
    blocksTransferred++;

    emit ftUpdateCounts();

    reply.append(IBM3270_AID_ENTER);
}

void FileTransfer::cutDataReq(DisplayScreen *screen, QByteArray &reply)
{
    qDebug() << "FileTransfer::cutDataReq - Outgoing data request";

    if (ftState == FileTransfer::FTState::ABORTING)
    {
        emit ftUpdateMessage("Transfer aborting...");
        cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABX);
        qDebug() << "FileTransfer::cutData - File Transfer is aborting";
        return;
    }   

    quint8 frameSeq = screen->getChar(IBM3270_FT_CUT_RDAT_FSEQ);
    ftBlock.clear();

    ftBlock.append(frameSeq);
    ftBlock.append(static_cast<char>(0x00)); // Checksum is filled after the data has been added.
    ftBlock.append(static_cast<char>(0x00)); // Data length is filled after
    ftBlock.append(static_cast<char>(0x00)); //      the data has been added.

    // Open file if it's the first time through
    if (!outgoingFile.isOpen())
    {
        quadrant = nullptr;
        lastwasCR = false;
        blocksTransferred = 0;
        bytesTransferred = 0;
        emit ftUpdateCounts();
        outgoingFile.setFileName(filename);
        if (!outgoingFile.open(QIODevice::ReadOnly)) {
            qDebug() << "cutData - File Transfer Open failed to open file for Read:" << filename;
            emit ftUpdateMessage(outgoingFile.errorString());
            cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABX);
            return;
        }
    }

    int pos = 0;

    // Allow for padding of 3 bytes for conversion between quadrants.
    while(pos <= IBM3270_FT_CUT_DATA_MAX - 3)
    {
        // Peek at the next byte in the file. If there is no next byte, we are at EOF, which is handled
        // after the loop, and must be done in a separate block.
        const QByteArray nextByte = outgoingFile.peek(1);
        if (nextByte.isEmpty())
            break;

        quint8 c = static_cast<quint8>(nextByte.at(0));

        // If the quadrant is not set, find the quadrant for the character. If the character is not in any
        // quadrant, abort the transfer because we don't know what to do with it. This should never happen
        // because all characters should be in one of the quadrants, but just in case, we handle it.
        if (quadrant == nullptr)
        {
            if (!(quadrant = findQuadrantByChar(c)))
            {
                qDebug() << "Failed to find quadrant for" << QString("%1").arg(c, 2, 16);
                cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABX);
                return;
            }
            ftBlock.append(quadrant->key);
            pos++;
        }

        // NULL can only be sent in quadrant 2. If the character is NULL and the current quadrant isn't 2,
        // switch to quadrant 2.
        if (!c)
        {
            if (quadrant != &quadrants[2])
            {
                quadrant = &quadrants[2];
                ftBlock.append(quadrant->key);
                pos++;
            }

            ftBlock.append(0xC1);
            pos++;
            lastwasCR = false;
            char consumed;
            outgoingFile.getChar(&consumed);
            continue;
        }

        // If ASCII and CRLF, and the last character wasn't a CR and this is a NL, add a CR
        if (ascii && crlf && !lastwasCR && c == '\n')
        {
            // Switch to quadrant 2 if not already there, and add the quadrant key
            if (quadrant != &quadrants[2])
            {
                quadrant = &quadrants[2];
                ftBlock.append(quadrant->key);
                pos++;
            }
            // Find CR in the quadrant and add it
            const void *cr = memchr(quadrant->map, '\r', 77);
            int index = (char*)cr - (char *)quadrant->map;
            quint8 ebc = a2e[(int)alphas[index]];
            ftBlock.append(ebc);
            pos++;
        }

        // Search for the character in the quadrant
        const void *chr = memchr(quadrant->map, c, 77);

        // If the character was found, map it and add the converted character
        // If it wasn't found, reset quadrant, add loop  around again
        if (chr != nullptr)
        {
            int index = (char*)chr - (char *)quadrant->map;
            quint8 ebc = a2e[(int)alphas[index]];
            ftBlock.append(ebc);
            pos++;
            lastwasCR = (c == '\r');
            char consumed;
            outgoingFile.getChar(&consumed);
        }
        else
        {
            quadrant = nullptr;
            continue;
        }
    }

    // Handle EOF (only processed if the file is at EOF and no data has been sent)
    if (outgoingFile.atEnd() && pos == 0)
    {
        qDebug() << "FileTransfer::cutDataReq - Outgoing data request - EOF";
        ftBlock.append(0x5C);
        ftBlock.append(0xA9);
        pos = 2;
    }

    qDebug() << "FileTransfer::cutDataReq - Outgoing data request - sent" << pos << "bytes";

    // Calculate length field
    ftBlock[IBM3270_FT_CUT_RESPONSE_LEN] = a2e[static_cast<quint8>((base64.at((pos >> 6) & 0x3f)).toLatin1())];
    ftBlock[IBM3270_FT_CUT_RESPONSE_LEN + 1] = a2e[static_cast<quint8>(base64.at(pos & 0x3F).toLatin1())];

    // Calculate checksum
    int checksum = 0;

    for (int i = IBM3270_FT_CUT_RESPONSE_SEND; i < ftBlock.size(); i++)
        checksum ^= static_cast<quint8>(ftBlock.at(i));

    // Store checksum in base64, converting to EBCDIC for the screen
    ftBlock[IBM3270_FT_CUT_RESPONSE_CKSM] =
        a2e[static_cast<quint8>(base64.at(checksum & 0x3F).toLatin1())];

    reply.append(IBM3270_AID_ENTER);
    reply.append(0x40); // Cursor address
    reply.append(0xC4);
    reply.append(IBM3270_SBA);
    reply.append(0x40); // Field address
    reply.append(0xC2);
    reply.append(IBM3270_FT_CUT_DATA);
    reply.append(ftBlock);

    bytesTransferred += pos;
    blocksTransferred++;  

    emit ftUpdateCounts();
}

/**
 * @brief   FileTransfer::cutControlCode - process the CUT mode control frame
 * @param   screen - the display screen object
 * @param   reply  - the reply buffer to send back to the host  
 * 
 * @details This function processes the CUT mode control frame. It extracts the control code from the screen,
 *          and handles the control code accordingly. It emits signals for success, failure, or completion
 *          of the file transfer. It also handles error messages from the host and sends appropriate replies.
 */
void FileTransfer::cutControlCode(DisplayScreen *screen, QByteArray &reply)
{
    QString msg;

    int code = (screen->getChar(IBM3270_FT_CUT_CTRLSTAT)) << 8 | screen->getChar(IBM3270_FT_CUT_CTRLSTAT + 1);

    qDebug() << "FileTranfer::cutControlCode - incoming control code" << QString("%1").arg(code, 4, 16);

//    screen->dumpDisplay();

    if (code != IBM3270_FT_CUT_CTRL_ACK)
    {
        for (int i = 0; i < 80; i++)
        {
            msg.append((QChar)e2a[screen->getChar(IBM3270_FT_CUT_CTRLMSGE + i)]);
        }

        msg = msg.trimmed();

        if (msg.size() > 0 && msg.last(1) == "$")
            msg = msg.left(msg.length() - 1);

        emit ftUpdateMessage(msg);

        qDebug() << "Message from host:" << msg;
    }

    switch(code)
    {
        case IBM3270_FT_CUT_CTRL_ACK:
            if (ftState == FTState::ABORTING)
            {
                if (outgoingFile.isOpen())
                    outgoingFile.close();
                if (incomingFile.isOpen())
                    incomingFile.cancelWriting();

                pendingDFTLF = false;
                setState(FTState::NONE);
                emit ftUpdateMessage(QString("Transfer cancelled"));
                emit ftFailure();
            }
            reply.append(IBM3270_AID_ENTER);
            quadrant = nullptr;
            break;
        case IBM3270_FT_CUT_CTRL_CMP:
            if (direction == Q3270::TransferDirection::Send) {
                outgoingFile.close();
            } else if (!incomingFile.commit()) {
                qDebug() << "ftClose - File Transfer Close failed to commit file:" << filename;
                emit ftUpdateMessage(incomingFile.errorString());
                cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABF);
                return;
            }
            qDebug() << "FileTransfer::cutControlCode - File Transfer complete - total bytes received" << bytesTransferred;
            reply.append(IBM3270_AID_ENTER);
            setState(FTState::NONE);
            emit ftSuccess();
            break;
        case IBM3270_FT_CUT_CTRL_ABF:
        case IBM3270_FT_CUT_CTRL_ABX:
            qDebug() << "FileTransfer::cutControlCode - File Transfer aborted - total bytes received" << bytesTransferred;
            emit ftUpdateCounts();
            if (msg == "")
                emit ftUpdateMessage(QString("File Transfer aborted"));
            reply.append(IBM3270_AID_ENTER);
            setState(FTState::NONE);
            emit ftSuccess();
            break;
        default:
            msg = QString("Unknown CUT control code %1").arg(code, 4, 16, '0');
            emit ftUpdateMessage(msg);
            cutAbort(screen, reply, IBM3270_FT_CUT_CTRL_ABX);
    }

}

/**
 * @brief   FileTransfer::cutRetransmit - process the CUT mode retransmit frame
 * @param   screen - the display screen object
 * @param   reply  - the reply buffer to send back to the host
 * 
 * @details This function processes the CUT mode retransmit frame. It sends an Enter AID to the host to request
 *          retransmission of the last data frame.
 */
void FileTransfer::cutRetransmit(QByteArray &reply)
{
    qDebug() << "FileTranfer::cutControlCode - retransmission request";

    if (ftMode != FTMode::CUT)
    {
        qDebug() << "FileTransfer::cutRetransmit - Not in CUT mode, ignoring retransmit request";
        return;
    }

    reply.append(IBM3270_AID_ENTER);
    reply.append(0x40); // Cursor address (4)0,(C)4
    reply.append(0xC4);
    reply.append(IBM3270_SBA);
    reply.append(0x40); // Field address (4)0,(C)2
    reply.append(0xC2);
    reply.append(IBM3270_FT_CUT_DATA);
    reply.append(ftBlock);
}

/**
 * @brief   FileTransfer::fromBase64 - convert a base64 character to its index
 * @param   b - the base64 character to convert
 * 
 * @details This function converts a base64 character to its index in the base64 character set. It uses the e2a
 *          table to convert from EBCDIC to ASCII, and then finds the index of the ASCII character in the base64 string.
 */
quint8 FileTransfer::fromBase64(quint8 b)
{
    quint8 ascii = e2a[b];
    int index = base64.indexOf((QChar)ascii);

    return (index < 0) ? 0 : (quint8)index;
}

/**
 * @brief   FileTransfer::cutAbort - send an abort response for CUT mode transfer
 * @param   screen - the display screen object
 * @param   reply  - the reply buffer to send back to the host
 * @param   reason - the reason code for the abort
 * 
 * @details This function appends the abort response at the CUT response field position. It is called when an error occurs during a CUT mode transfer,
 *          such as a file open failure or an unmapped character.
 */
void FileTransfer::cutAbort(DisplayScreen *screen, QByteArray &reply, quint16 reason)
{
    qDebug() << "FileTransfer::cutAbort - aborting with" << QString("%1").arg(reason, 4, 16);

    quint8 bh = (reason >> 8) & 0xFF;
    quint8 bl = reason & 0xFF;
    quint8 frameType = screen->getChar(IBM3270_FT_FRAMETYPE);
    quint8 frameSeq = screen->getChar(frameType == IBM3270_FT_CUT_DATAREQ
                                          ? IBM3270_FT_CUT_RDAT_FSEQ
                                          : IBM3270_FT_CUT_DATA_FSEQ);

    reply.append(IBM3270_AID_F2);
    reply.append(0x40); // Cursor address
    reply.append(0xC4);

    // Position of only enterable field on the CUT screen, at IBM3270_FT_CUT_RESP_FTYP
    reply.append(IBM3270_SBA);
    screen->addPosToBuffer(reply, IBM3270_FT_CUT_RESP_FTYP);

    // Four bytes of response: CUT control code, CUT data frame sequence, and the two bytes of the reason code
    reply.append(IBM3270_FT_CUT_CONTROL);
    reply.append(frameSeq);
    reply.append(static_cast<char>(bh));
    reply.append(static_cast<char>(bl));
}

/**
 * @brief   FileTransfer::findQuadrantByCode - find the quadrant by its code
 * @param   b - the code of the quadrant to find
 * 
 * @details This function searches the quadrants array for a quadrant with the specified code. It returns a pointer
 *         to the quadrant if found, or nullptr if not found.
 */
const FileTransfer::Quadrant* FileTransfer::findQuadrantByCode(quint8 b)
{
    for (int q = 0; q < 4; q++)
    {
        if (b == quadrants[q].key)
            return &(quadrants[q]);
    }

    return nullptr;
}

/** 
 * @brief   FileTransfer::findQuadrantByChar - find the quadrant by a character
 * @param   c - the character to find the quadrant for
 * 
 * @details This function searches the quadrants array for a quadrant that contains the specified character in its
 *          map. It returns a pointer to the quadrant if found, or returns nullptr.
 */
const FileTransfer::Quadrant* FileTransfer::findQuadrantByChar(quint8 c)
{
    for (int q = 0; q < 4; q++)
    {
        for (int i = 0; i < 77; i++)
        {
            if (c == quadrants[q].map[i])
            {
//                qDebug() << QString("Found quadrant for %1 is %2").arg(c, 2, 16).arg(quadrants[q].key, 2, 16);
                return &(quadrants[q]);
            }
        }
    }
    return nullptr;
}

/** 
 * @brief   FileTransfer::getMode - get the current file transfer mode
 * @details This function returns the current file transfer mode. It is used
 *          to determine if the transfer is in DFT or CUT mode.
 */
FileTransfer::FTMode FileTransfer::getMode()
{
    return ftMode;
}

void FileTransfer::ftCancel()
{
    qDebug() << "FileTransfer::ftCancel - transfer cancelled";
    emit ftUpdateMessage("File Transfer aborting...");

    if (ftMode == FTMode::DFT) {
        if (outgoingFile.isOpen())
            outgoingFile.close();
        if (incomingFile.isOpen())
            incomingFile.cancelWriting();
        pendingDFTLF = false;
    }

    setState(FTState::ABORTING);
}

void FileTransfer::finishDFTCancellation(QByteArray &reply, quint16 errorType)
{
    reply.append(IBM3270_AID_SF);
    reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                     .addHalfWord(errorType)
                     .addHalfWord(IBM3270_FT_ERROR_HDR)
                     .addHalfWord(IBM3270_FT_ERROR_CMD)
                     .toSF());

    pendingDFTLF = false;
    setState(FTState::NONE);
    emit ftUpdateMessage(QString("Transfer cancelled"));
    emit ftFailure();
}

void FileTransfer::connectionLost()
{
    if (ftState == FTState::NONE)
        return;

    if (outgoingFile.isOpen())
        outgoingFile.close();
    if (incomingFile.isOpen())
        incomingFile.cancelWriting();

    pendingDFTLF = false;
    setState(FTState::NONE);
    emit ftUpdateMessage(QString("Connection lost; transfer stopped"));
    emit ftFailure();
}

bool FileTransfer::isDFTEnabled()
{
    return dftEnabled;
}

quint16 FileTransfer::getDFTBlockSize()
{
    return dftBlockSize;
}