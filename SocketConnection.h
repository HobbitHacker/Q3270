// SocketConnection.h

#ifndef SOCKETCONNECTION_H
#define SOCKETCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#include <arpa/telnet.h>

#include "Q3270.h"
#include "ProcessDataStream.h"

class QHostAddress;

class SocketConnection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SocketConnection)

    public:
        explicit SocketConnection(QString termName);
        ~SocketConnection();

        void sendResponse(QByteArray &b);

    public slots:
        void connectMainframe(const QHostAddress &address, quint16 port, QString luName, ProcessDataStream *d);
        void disconnectMainframe();
        void closed();

    signals:
        void disconnected3270();
        void dataStreamComplete(QByteArray &b, bool tn3270e);
        void error(QAbstractSocket::SocketError socketError);

    private slots:
        void onReadyRead();

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
        QTcpSocket *dataSocket;
        QDataStream dataStream;
        ProcessDataStream *displayDataStream;

        QString termName;
        QString luName;

        QByteArray incomingData;
        QByteArray subNeg;

        void sendData();

        void datastreamReceived(const QJsonObject &doc);

        void processSubNegotiation();

        const char *tn3270e_functions_strings[5] = {"BIND_IMAGE", "DATA_STREAM_CTL", "RESPONSES", "SCS_CTL_CODES", "SYSREQ"};

        char tn32703_functions_flags[5];

        void dump(QByteArray &a, QString title);
};

#endif // SOCKETCONNECTION_H
