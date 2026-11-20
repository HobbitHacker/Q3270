/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2026 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "ProcessDataStream.h"
#include "StructuredField.h"
#include "FileTransfer/FileTransfer.h"
#include "Terminal.h"

/**
 * @brief   ProcessDataStream::WSFoutbound3270DS - The 3270 Structured Field Outbound 3270DS
 *
 * @details The Outbound 3270DS is used to direct output to a specific partition. There are
 *          four possible incoming (outbound from the host) operations:
 *
 *          @li Write
 *          @li Erase/Write
 *          @li Erase/Write Alternate
 *          @li Erase All Unprotected
 *
 *          These operations can be directed to a specific partition after the Create Partition
 *          structured field has been used.
 *
 * @note    Q3270 supports only WRITE currently.
 */
void ProcessDataStream::WSFoutbound3270DS()
{
    if (wsfLen < 6)
    {
        qDebug() << "Outbound 3270DS: field is too short for partition, command, and WCC";
        return;
    }

    QByteArray::Iterator fieldEnd = buffer + (wsfLen - 3);
    quint8 partition = nextBufferByte();
    quint8 cmnd = nextBufferByte();

    qDebug() << "[Outbound 3270DS partition" << partition << "command" << cmnd << "length" << wsfLen << "]";

    switch(cmnd)
    {
        case IBM3270_W:
            qDebug() << "Write ";
            processWCC();
            while (buffer < fieldEnd)
                processOrders();
            break;
        default:
            qDebug() << QString("\n[** Unimplemented Outbound3270DS command %02X - Ignored **]\n\n").arg(cmnd, 2, 16);
    }
}

/**
 * @brief   ProcessDataStream::WSFreset - The 3270 Structured Field Reset
 *
 * @details The Reset is used to reset the terminal to its power on state.
 */
void ProcessDataStream::WSFreset()
{
    printf("\n\n[** Reset Partition %02X - Not Implemented **]\n\n", nextBufferByte());
    return;
}

/**
 * @brief   ProcessDataStream::WSFreadPartition - The 3270 Structured Field Read Partition
 *
 * @details The Read Partition function allows the host to read a specific partition or to
 *          query the device to report functions that it supports.
 *
 * @note    Q3270 only supports the Query function.
 */
void ProcessDataStream::WSFreadPartition()
{
    quint8 partition = nextBufferByte();
    quint8 type = nextBufferByte();

    printf("ReadPartition %d - type %2.2X\n", partition, type);

    reply.append((uchar) IBM3270_AID_SF);

    replySummary();
}

/**
 * @brief    ProcessDataStream::replySummary - The 3270 Structured Field Query Reply (Summary)
 * @param    queryReply - the buffer to add the reply to
 *
 * @details  The Query Reply (Summary) structured field is used to inform the host of a summary of
 *           the functions that the device supports.
 *
 *           Here, the reply summary also includes, as structured fields following the summary, the
 *           device capabililties.
 *
 *           It is this function that allows the host the utilise the dynamic screen sizes.
 */
void ProcessDataStream::replySummary()
{
    quint8 width  = terminal->terminalWidth(true);
    quint8 height = terminal->terminalHeight(true);

    QSizeF screenSizeMM = QGuiApplication::primaryScreen()->physicalSize();
    QSizeF screenSizePix = QGuiApplication::primaryScreen()->size();

    int xmm = screenSizeMM.width();
    int ymm = screenSizeMM.height();

    int x = screenSizePix.width();
    int y = screenSizePix.height();

    StructuredField sf = StructuredField(IBM3270_SF_QUERYREPLY)
                    .addByte(IBM3270_SF_QUERYREPLY_SUMMARY)
                    .addByte(IBM3270_SF_QUERYREPLY_COLOUR)
                    .addByte(IBM3270_SF_QUERYREPLY_IMPPARTS)
                    .addByte(IBM3270_SF_QUERYREPLY_USABLE)
                    .addByte(IBM3270_SF_QUERYREPLY_CHARSETS)
                    .addByte(IBM3270_SF_QUERYREPLY_HIGHLIGHT);

    if (fileTransfer.isDFTEnabled())
        sf.addByte(IBM3270_SF_QUERYREPLY_DDM);

    reply.append(sf.toSF());

    reply.append(StructuredField(IBM3270_SF_QUERYREPLY)
                    .addByte(IBM3270_SF_QUERYREPLY_COLOUR)
                    .addByte(0x00) /* Flags:  x.xxxxxx - Reserved
                                              .0...... - Printer Only - Black ribbon not loaded
                                              .1...... - Printer Only - Black ribbon loaded
                                   */
                    .addByte(0x08) /* Number of colours, plus default colour */
                    .addBytes({0x00, 0xF4}) /* Neutral (black on screen, white on print) */
                    .addBytes({0xF1, 0xF1}) /* Blue */
                    .addBytes({0xF2, 0xF2}) /* Red */
                    .addBytes({0xF3, 0xF3}) /* Magenta */
                    .addBytes({0xF4, 0xF4}) /* Green */
                    .addBytes({0xF5, 0xF5}) /* Cyan */
                    .addBytes({0xF6, 0xF6}) /* Yellow */
                    .addBytes({0xF7, 0xF7}) /* Neutral (white on screen, black on print) */
                    .toSF());

    reply.append(StructuredField(IBM3270_SF_QUERYREPLY)
                    .addByte(IBM3270_SF_QUERYREPLY_IMPPARTS)
                    .addByte(0x00) /* Reserved */
                    .addByte(0x00) /* Reserved */
                    .addByte(0x0B) /* Data Length */
                    .addByte(0x01) /* Implicit Partition Sizes */
                    .addByte(0x00) /* Reserved */
                    .addHalfWord(0x0080) /* Default Width in characters */
                    .addHalfWord(0x0024) /* Default Height in characters */
                    .addHalfWord(width) /* Alternate Width in characters */
                    .addHalfWord(height) /* Alternate Height in characters */
                    .toSF());

    reply.append(StructuredField(IBM3270_SF_QUERYREPLY)
                    .addByte(IBM3270_SF_QUERYREPLY_HIGHLIGHT)
                    .addByte(0x04) /* Number of pairs */
                    .addBytes({0x00, 0xF0}) /* Default */
                    .addBytes({0xF1, 0xF1}) /* Blink */
                    .addBytes({0xF2, 0xF2}) /* Reverse */
                    .addBytes({0xF4, 0xF4}) /* Uscore */
                    .toSF());

    reply.append(StructuredField(IBM3270_SF_QUERYREPLY)
                    .addByte(IBM3270_SF_QUERYREPLY_USABLE)
                    .addByte(0x01) /* 12/14 bit addressing */
                    .addByte(0x00) /* Nothing enabled */
                    .addHalfWord(width) /* Columns */
                    .addHalfWord(height) /* Rows */
                    .addByte(0x01) /* Units in mm */
                    .addHalfWord(xmm)
                    .addHalfWord(x) /* Xr numerator/denominator */
                    .addHalfWord(ymm)
                    .addHalfWord(y) /* Yr numerator/denominator */
                    .addByte(CELL_WIDTH) /* W */
                    .addByte(CELL_HEIGHT) /* H */
                    .addHalfWord(width * height) /* Screen buffer size */
                    .toSF());

    reply.append(StructuredField(IBM3270_SF_QUERYREPLY)
                    .addByte(IBM3270_SF_QUERYREPLY_CHARSETS)
                    .addByte(0x82) /* Flags: x....... - ALT (GE support)
                                             .x...... - MULTID (multiple LCIDs support)
                                             ..x..... - LOADABLE (loadable PS support)
                                             ...x.... - EXT (loadable PS extended support)
                                             ....x... - MS (>1 character slot size support)
                                             .....x.. - CH2 (DBCS support)
                                             ......x. - GF (CGCSGID present)
                                             .......x - Reserved */
                    .addByte(0x00) /* Reserved */
                    .addBytes({0x04, 0x03}) /* Default Character Slot Width and Height */
                    .addBytes({0x00, 0x00, 0x00, 0x00}) /* Load PS Format Types Supported (none) */
                    .addByte(0x07) /* Length of Descriptor Areas */
                    /* Descriptor Set 1 */
                    .addByte(0x00) /* Char Set */
                    .addByte(0x10) /* Flags: No LCID Compare */
                    .addByte(0x00) /* Local Character Set ID */
                    .addBytes({0x02, 0xB9}) /* Character Set Number */
                    .addBytes({0x00, 0x25}) /* Code Page 037 */
                    /* Descriptor Set 2 */
                    .addByte(0x01) /* Char Set */
                    .addByte(0x10) /* Flags: No LCID Compare */
                    .addByte(0xF1) /* Local Character Set ID */
                    .addBytes({0x03, 0xC3}) /* Character Set Number */
                    .addBytes({0x01, 0x36}) /* Code Page 310 */
                    .toSF());

    // reply.append(StructuredField(IBM3270_SF_QUERYREPLY)
    //                 .addByte(IBM3270_SF_QUERYREPLY_PARTS)
    //                 .addByte(0x02) /* Number of Partitions */
    //                 .addBytes({0xFF, 0xFE}) /* Storage Allowed */
    //                 .addByte(0x00) /* Partition Flags: x....... - SCROLL
    //                                                    .xx..... - Reserved
    //                                                    ...x.... - AP (All Points Addressable)
    //                                                    ....x... - PROT (Partition Protection)
    //                                                    .....x.. - COPY (Presentation Space Local Copy Supported)
    //                                                    ......x. - MODIFY (Modify Partition Supported)
    //                                                    .......x - Reserved */
    //                 .toSF());


    if (fileTransfer.isDFTEnabled())
        reply.append(StructuredField(IBM3270_SF_QUERYREPLY)
                    .addByte(IBM3270_SF_QUERYREPLY_DDM)
                    .addBytes({0x00, 0x00}) /* Reserved */
                    .addHalfWord(fileTransfer.getDFTBlockSize()) /* Limit In (LIMIN) */
                    .addHalfWord(fileTransfer.getDFTBlockSize()) /* Limit Out (LIMOUT) */
                    .addByte(0x01) /* Number of subsets supported */
                    .addByte(0x01) /* Subset number */
                    .toSF());
}

/**
 * @brief   ProcessDataStream::WSFtransfer - The 3270 Structured Field File Transfer
 *
 * @details The Transfer structured field is used to transfer data between the host and the terminal.
 *          This is typically used for file transfer operations, such as sending or receiving files.
 */
void ProcessDataStream::WSFtransfer()
{
    // TODO: To be resinstated later
    /*
    if (!fileTransfer.isExpected()) {
        qDebug() << "WSFtransfer: File Transfer not expected - ignoring";
        return;
    }
    */

    quint16 trtype = nextBufferHalfWord();

    qDebug() << "WSFtransfer: type" << QString::number(trtype, 16);

    if (!fileTransfer.isDFTEnabled()) {
        qDebug() << "WSFtransfer: DFT is disabled for this transfer; structured field ignored";
        return;
    }

    switch (trtype) {
        case IBM3270_FT_OPEN_REQ:
            fileTransfer.open(buffer, reply, wsfLen);
            lastAID = IBM3270_AID_SF;
            break;
        case IBM3270_FT_INSERT_REQ:
            fileTransfer.insertReq();
            break;
        case IBM3270_FT_INSERT_DATA:
            fileTransfer.insertData(buffer, reply);
            lastAID = IBM3270_AID_SF;
            break;
        case IBM3270_FT_SETCUR_REQ:
            fileTransfer.setCursorReq();
            break;
        case IBM3270_FT_GET_REQ:
            fileTransfer.getData(reply);
            lastAID = IBM3270_AID_SF;
            break;
        case IBM3270_FT_CLOSE_REQ:
            fileTransfer.close(reply);
            lastAID = IBM3270_AID_SF;
            break;
        default:
            qDebug() << QString("[** WSF Transfer - Not Implemented %1 **]").arg(trtype, 4, 16);
    }
}
