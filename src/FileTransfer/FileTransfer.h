/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2026 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QObject>
#include <QSaveFile>
#include <QFile>

#include "Q3270.h"

class FileTransfer : public QObject
{
    Q_OBJECT

    public:
        FileTransfer();

        void setExpected(bool expected);

        void open(QByteArray::Iterator &buffer, QByteArray &reply, int wsfLen);
        void insertReq();
        void insertData(QByteArray::Iterator &buffer, QByteArray &reply);
        void setCursorReq();
        void getData(QByteArray &reply);
        void close(QByteArray &reply);

        void setProperties(QString filemame, Q3270::TransferDirection direction, bool ascii, bool crlf);

        quint64 getBytes();
        quint64 getBlocks();

        bool isExpected();

    signals:

        void ftStarted();
        void ftUpdateCounts();
        void ftUpdateMessage(const QString &message);
        void ftComplete();
        void ftAbort();

        void ftSuccess();
        void ftFailure();

    private:
        QString filename;

        QSaveFile incomingFile;
        QFile outgoingFile;

        Q3270::TransferDirection direction;

        quint64 bytesTransferred;
        quint64 blocksTransferred;

        bool ftExpected;
        bool ftMsg;

        bool ascii;
        bool crlf;

        void ftDataAck(QByteArray &reply);
        void ftAbort(QString msg, quint16 abortType, QByteArray &reply);

};

#endif // FILETRANSFER_H
