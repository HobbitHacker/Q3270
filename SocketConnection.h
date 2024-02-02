/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

        void sendResponse(QByteArray &b);

    public slots:
        void connectMainframe(QString &address, quint16 port, QString luName, ProcessDataStream *d);
        void disconnectMainframe();
        void opened();
        void closed();

    signals:
        void connectionStarted();
        void connectionEnded(QString message = "");
        void dataStreamComplete(QByteArray &b, bool tn3270e);

    private slots:
        void onReadyRead();
        void sslErrors(const QList<QSslError> &errors);
        void socketStateChanged(QAbstractSocket::SocketState state);
        void error(QAbstractSocket::SocketError socketError);

    private:

        enum TelnetState {
            TELNET_STATE_DATA,
            TELNET_STATE_IAC,
            TELNET_STATE_IAC_DO,
            TELNET_STATE_IAC_DONT,
            TELNET_STATE_IAC_WILL,
            TELNET_STATE_IAC_WONT,
            TELNET_STATE_IAC_SB,
            TELNET_STATE_SB,
            TELNET_STATE_SB_IAC,
            TELNET_STATE_SB_TTYPE,
            TELNET_STATE_SB_TTYPE_SEND,
            TELNET_STATE_SB_TTYPE_SEND_IAC,
            TELNET_STATE_SB_TN3270E,
            TELNET_STATE_SB_TN3270E_SEND,
            TELNET_STATE_SB_TN3270E_SEND_DEVICE_TYPE
        };

        bool tn3270e_Mode;

        TelnetState telnetState;
//        QSslSocket *dataSocket;
        QTcpSocket *dataSocket;
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
