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

#ifndef TELOPT_TN3270E
#define TELOPT_TN3270E 0x28 /* TN3270 Extended */

#define TN3270E_ASSOCIATE    0x00
#define TN3270E_CONNECT      0x01
#define TN3270E_DEVICE_TYPE  0x02
#define TN3270E_FUNCTIONS    0x03
#define TN3270E_IS           0x04
#define TN3270E_REASON       0x05
#define TN3270E_REJECT       0x06
#define TN3270E_REQUEST      0x07
#define TN3270E_SEND         0x08

#define TN3270E_REASON_CONN_PARTNER      0x00
#define TN3270E_REASON_DEVICE_IN_USE     0x01
#define TN3270E_REASON_INV_ASSOCIATE     0x02
#define TN3270E_REASON_INV_NAME          0x03
#define TN3270E_REASON_INV_DEVICE_TYPE   0x04
#define TN3270E_REASON_TYPE_NAME_ERROR   0x05
#define TN3270E_REASON_UNKNOWN_ERROR     0x06
#define TN3270E_REASON_UNSUPPORTED_REQ   0x07

#define TN3270E_FUNCTION_BIND_IMAGE      0x00
#define TN3270E_FUNCTION_DATA_STREAM_CTL 0x01
#define TN3270E_FUNCTION_RESPONSES       0x02
#define TN3270E_FUNCTION_SCS_CTL_CODES   0x03
#define TN3270E_FUNCTION_SYSREQ          0x04

#define TN3270E_DATATYPE_3270_DATA       0x00
#define TN3270E_DATATYPE_SCS_DATA        0x01
#define TN3270E_DATATYPE_RESPONSE        0x02
#define TN3270E_DATATYPE_BIND_IMAGE      0x03
#define TN3270E_DATATYPE_UNBIND          0x04
#define TN3270E_DATATYPE_NVT_DATA        0x05
#define TN3270E_DATATYPE_REQUEST         0x06
#define TN3270E_DATATYPE_SSCP_LU_DATA    0x07
#define TN3270E_DATATYPE_PRINT_EOJ       0x08

#endif

#include "ProcessDataStream.h"

class QHostAddress;

class SocketConnection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SocketConnection)

    public:
        explicit SocketConnection(QString termName);

        void sendResponse(Buffer *b);

    public slots:
        void connectMainframe(const QHostAddress &address, quint16 port, QString luName, ProcessDataStream *d);
        void disconnectMainframe();

    private slots:
        void onReadyRead();


    signals:
        void connected();
        void dataStreamComplete(Buffer *b);
        void disconnected();
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
        QTcpSocket *dataSocket;
        QDataStream dataStream;
        ProcessDataStream *displayDataStream;

        QString termName;
        QString luName;

        Buffer *incomingData;
        Buffer *subNeg;

        void sendData();

        void datastreamReceived(const QJsonObject &doc);

        void processSubNegotiation();
        void processBuffer();

        const char *tn3270e_functions_strings[5] = {"BIND_IMAGE", "DATA_STREAM_CTL", "RESPONSES", "SCS_CTL_CODES", "SYSREQ"};

        char tn32703_functions_flags[5];
};

#endif // SOCKETCONNECTION_H
