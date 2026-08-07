/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2026 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */
#include <QString>
#include <QDebug>

#include "ProcessDataStream/ProcessDataStream.h"
#include "ProcessDataStream/StructuredField.h"

void ProcessDataStream::ftOpen()
{
    QString packetType;

    qDebug() << "ftOpen - File Transfer Open command received - length" << wsfLen;

    // TODO: FileTransfer - "open" packet sizes different to 32
    if (wsfLen == 0x23) {
        packetType = QString::fromLatin1(reinterpret_cast<const char *>(buffer + 23), 7);
        qDebug() << "FTopen() - Packet type:" << packetType;
    }

    if (packetType.slice(0, 6) == "FT:MSG")
        ftMsg = true;
    else {
        ftBytes = 0;
        ftMsg = false;
        emit ftStarted();
    }

    // Open sets record count to 1
    ftRecordCount = 1;

    reply.append(IBM3270_AID_SF);

    reply.append(StructuredField(IBM3270_WSF_TRANSFER).
                 addHalfWord(IBM3270_FT_OPEN_ACK).
                 toSF());
}

// InsertReq doesn't actually do anything
void ProcessDataStream::ftInsertReq()
{
}

void ProcessDataStream::ftInsertData()
{
    qDebug() << "ftInsertData - File Transfer Insert Data command received - length" << wsfLen;

    // Skip past C0 80 61 static bytes
    // These, according to x3270, are compression indicator (C080) and start of data flags (61)
    buffer += 3;

    quint16 pktLen = nextBufferHalfWord();

    qDebug() << "ftInsertData - pktLen = " << pktLen << " bytes";

    if (ftMsg) {
        QString msg = QString::fromLatin1(reinterpret_cast<const char *>(buffer), pktLen);
        int dollar = msg.indexOf('$');
        if (dollar > 0)
            msg = msg.left(dollar);
        qDebug() << msg << ftBytes << "bytes received";
        emit ftUpdateMessage(msg);
        ftDataAck();
        if (msg.left(7) == "TRANS03")
            qDebug() << "Success!";
        else
            qDebug() << "Failure!";

        // TODO: Handle messages
        return;
    }

    if (pktLen > 0) {
        /* Write data out */
        ftBytes += pktLen;
        emit ftUpdateCounts(ftBytes, ftRecordCount);
    }

    ftDataAck();
}

void ProcessDataStream::ftClose()
{
    qDebug() << "ftClose - File Transfer Close command received - length" << wsfLen;

    reply.append(IBM3270_AID_SF);

    reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                     .addHalfWord(IBM3270_FT_CLOSE_ACK)
                     .toSF());

    emit ftComplete();

    qDebug() << "ftClose - File Transfer complete - total bytes received" << ftBytes;
}

void ProcessDataStream::ftDataAck()
{
    reply.append(IBM3270_AID_SF);

    reply.append(StructuredField(IBM3270_WSF_TRANSFER)
                     .addHalfWord(IBM3270_FT_NORMAL_ACK)
                     .addHalfWord(IBM3270_FT_RECNUM_HDR)
                     .addWord(ftRecordCount)
                     .toSF());

    ftRecordCount++;
}
