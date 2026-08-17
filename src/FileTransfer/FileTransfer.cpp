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

#include "FileTransfer/FileTransfer.h"
#include "ProcessDataStream/StructuredField.h"

/*
 * IND$FILE Processing
 *
 * IND$FILE has a number of commands:
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
 */

/**
 * @brief   FileTransfer::FileTransfer - initialise the file transfer object
 *
 * @details The file transfer object is used to handle the IND$FILE structured fields. It is signalled
 *          when a file transfer is started, and processes the requests for reading or writing local files.
 */
FileTransfer::FileTransfer()
{
    ftExpected = false;
}

/**
 * @brief   FileTransfer::setProperties - set the properties for the file transfer
 * @param   filename  - the local file name to read/write
 * @param   direction - the direction of the transfer (send or receive)
 * @param   ascii     - true for ASCII transfer, false for binary
 * @param   crlf      - true to convert CRLF to LF on receive, false otherwise
 *
 * @details This function is called before starting a file transfer to set the properties of
 *          the transfer.
 */
void FileTransfer::setProperties(QString filename, Q3270::TransferDirection direction, bool ascii, bool crlf)
{
    this->filename = filename;
    this->direction = direction;
    this->ascii = ascii;
    this->crlf = crlf;
}

/**
 * @brief   FileTransfer::open - process the Open Req structured field
 * @param   buffer - the incoming data buffer
 * @param   reply  - the reply buffer to send back to the host
 * @param   wsfLen - the length of the structured field
 *
 * @details This function processes the Open Req structured field. It checks the packet type and opens
 *          the local file for reading or writing, depending on the direction of the transfer. It detects
 *          whether the OPEN request is for data or for a message.
 */
void FileTransfer::open(QByteArray::Iterator &buffer, QByteArray &reply, int wsfLen)
{
    QString packetType;

    qDebug() << "ftOpen - File Transfer Open command received";

    switch (wsfLen) {
        // Receive
        case 0x23:
            packetType = QString::fromLatin1(reinterpret_cast<const char *>(buffer + 23), 7);
            qDebug() << "FTopen() - Packet type:" << packetType;
            break;
        // Send
        case 0x29:
            {
                quint16 hi = static_cast<quint8>(*buffer++);
                quint16 lo = static_cast<quint8>(*buffer++);
                quint16 pktLen = (hi << 8) | lo;
                packetType = QString::fromLatin1(reinterpret_cast<const char *>(buffer + 29), 7);
                qDebug() << "FTopen() - Packet type:" << packetType;
            }
            break;
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
                break;
            default:
                qDebug() << "ftOpen - File Transfer Open unknown direction";
        }
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
        // TRANS03 is complete, TRANS04 is complete with segemented records
        if (msg.left(7) == "TRANS03" || msg.left(7) == "TRANS04")
            emit ftSuccess();
        else
            emit ftFailure();
        return;
    }

    if (pktLen > 0) {
        // If this is an ASCII transfer with CRLF, remove the CR and EOF characters
        if (ascii && crlf) {
            QByteArray converted;
            for (int i = 0; i < pktLen; i++) {
                uint c = buffer[i];
                if (c == 0x1A || c == '\r') {
                    // Skip EOF character and carriage return
                    continue;
                }
                converted.append(c);
            }
            incomingFile.write(converted, converted.length());
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
    if (outgoingFile.atEnd()) {
        reply.append(IBM3270_AID_SF);
        reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                         .addHalfWord(IBM3270_FT_ERROR_GET)
                         .addHalfWord(IBM3270_FT_ERROR_HDR)
                         .addHalfWord(IBM3270_FT_ERROR_EOF)
                         .toSF());
        qDebug() << "ftGetData - File Transfer Get Data end of file reached";
        return;
    }

    // TODO: Should be dynamic, as per replySummary
    QByteArray data = outgoingFile.read(16384 - 27); // 27 bytes for header and SF

    if (outgoingFile.error() != QFile::NoError) {
        ftAbort(outgoingFile.errorString(), IBM3270_FT_ERROR_GET, reply);
        return;
    }

    if (crlf) {
        QByteArray converted;
        for (int i = 0; i < data.length(); i++) {
            uint c = data[i];
            if (c == '\n') {
                converted.append('\r');
                converted.append('\n');
            } else {
                converted.append(c);
            }
        }
        data = converted;
    }

    reply.append(IBM3270_AID_SF);

    StructuredField sendData = StructuredField(IBM3270_WSF_TRANSFER)
                     .addHalfWord(IBM3270_FT_GET_ACK)
                     .addHalfWord(IBM3270_FT_RECNUM_HDR)
                     .addWord(blocksTransferred)
                     .addHalfWord(IBM3270_FT_NOTCOMP)
                     .addByte(IBM3270_FT_DATA_START)
                     .addHalfWord(data.length() + 5); // Add 5 for the 5 bytes of header

    for (int i = 0; i < data.length(); i++) {
       qDebug() << "getData - data[" << i << "] = " << QString::number(data[i], 16);
       sendData.addByte(data[i]);
    }

    reply.append(sendData.toSF());

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

    emit ftComplete();

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

    emit ftAbort();
}

/**
 * @brief   FileTransfer::reset - reset the file transfer state
 *
 * @details This function resets the file transfer state. It is called when a new file transfer
 *          is expected and when one is complete.
 */
void FileTransfer::setExpected(bool exp)
{
    ftExpected = exp;
    if (ftExpected)
        emit ftStarted();
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
bool FileTransfer::isExpected()
{
    return ftExpected;
}
