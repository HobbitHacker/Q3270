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
public slots:
    void connectMainframe(const QHostAddress &address, quint16 port);
//    void login(const QString &userName);
//    void sendMessage(const QString &text);
    void disconnectMainframe();
private slots:
    void onReadyRead();
signals:
    void connected();
	void dataStreamComplete();
//    void loggedIn();
//    void loginError(const QString &reason);
    void disconnected();
//    void messageReceived(const QString &sender, const QString &text);
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
    QTcpSocket *m_clientSocket;
	DisplayDataStream *displayDataStream;
    void datastreamReceived(const QJsonObject &doc);
	void printHex(char *s);
};

#endif // SOCKETCONNECTION_H
