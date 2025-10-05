/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef SOCKETCONNECTION_H
#define SOCKETCONNECTION_H

#include <QDebug>

#include <QObject>
#include <QSslSocket>
#include <QDataStream>

#include <QSslConfiguration>
#include <QSslCipher>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#include <arpa/telnet.h>

#include "ProcessDataStream.h"

class QHostAddress;

class SocketConnection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SocketConnection)

    public:

        explicit SocketConnection(int modelType);
        ~SocketConnection();

        void setSecure(bool s);
        void setVerify(bool v);
        void sendResponse(QByteArray &b);

        QList<QSslCertificate> getCertDetails();

    public slots:
        void connectMainframe(const QString &address, quint16 port, const QString luName, ProcessDataStream *d);
        void disconnectMainframe();
        void opened();
        void closed();

    signals:
        void connectionStarted();
        void connectionEnded(QString message = "");
        void dataStreamComplete(QByteArray &b, bool tn3270e);
        void encryptedConnection(Q3270::Encryption e);

    private slots:
        void onReadyRead();
        void sslErrors(const QList<QSslError> &errors);
        void socketStateChanged(QAbstractSocket::SocketState state);
        void error(QAbstractSocket::SocketError socketError);
        void socketEncrypted();

    private:

        bool tn3270e_Mode;

        bool secureMode;
        bool verifyCerts;
        bool certErrors;

        Q3270::TelnetState telnetState;
        QSslSocket *dataSocket;
//        QTcpSocket *dataSocket;
        QDataStream dataStream;
        ProcessDataStream *displayDataStream;

        QString termName;
        QString luName;

        QByteArray incomingData;
        QByteArray subNegotiationBuffer;

        void processSubNegotiation();

        const char *tn3270e_functions_strings[5] = { "BIND_IMAGE", "DATA_STREAM_CTL", "RESPONSES", "SCS_CTL_CODES", "SYSREQ" };

        QString tn3270e_terminal_types[5] = { "IBM-3279-2-E", "IBM-3279-3-E", "IBM-3279-4-E", "IBM-3279-5-E", "IBM-DYNAMIC" };

        char tn32703_functions_flags[5];

        void dump(QByteArray &a, QString title);
};

#endif // SOCKETCONNECTION_H
