// SocketConnection.h

#ifndef SOCKETCONNECTION_H
#define SOCKETCONNECTION_H

#include <QObject>
#include <QTcpSocket>

#include "DisplayDataStream.h"

class QHostAddress;

class SocketConnection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SocketConnection)

    public:
    explicit SocketConnection(QObject *parent = nullptr);

    void sendResponse(Buffer *b);



public slots:
    void connectMainframe(const QHostAddress &address, quint16 port, DisplayDataStream *d);
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
		TELNET_STATE_SB_IAC,
		TELNET_STATE_SB_TTYPE,
		TELNET_STATE_SB_TTYPE_SEND,
		TELNET_STATE_SB_TTYPE_SEND_IAC
	};
	TelnetState telnetState;
    QTcpSocket *dataSocket;
    QDataStream dataStream;
	DisplayDataStream *displayDataStream;
    Buffer *incomingData;

    void sendData();

    void datastreamReceived(const QJsonObject &doc);

};

#endif // SOCKETCONNECTION_H
