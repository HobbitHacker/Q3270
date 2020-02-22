#include "SocketConnection.h"
#include "DisplayDataStream.h"

#include <QTcpSocket>
#include <QDataStream>


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>

#include <arpa/telnet.h>

SocketConnection::SocketConnection(QObject *parent)
    : QObject(parent)
	, m_clientSocket(new QTcpSocket(this))
	, displayDataStream(new DisplayDataStream())
{
	telnetState = TELNET_STATE_DATA;
	
    // Forward the connected and disconnected signals
    connect(m_clientSocket, &QTcpSocket::connected, this, &SocketConnection::connected);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &SocketConnection::disconnected);
    // connect readyRead() to the slot that will take care of reading the data in
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &SocketConnection::onReadyRead);
    // Forward the error signal, QOverload is necessary as error() is overloaded, see the Qt docs
    connect(m_clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &SocketConnection::error);
    // Reset the m_loggedIn variable when we disconnec. Since the operation is trivial we use a lambda instead of creating another slot
//    connect(m_clientSocket, &QTcpSocket::disconnected, this, [this]()->void{m_loggedIn = false;});
}


void SocketConnection::disconnectMainframe()
{
    m_clientSocket->disconnectFromHost();
}

void SocketConnection::connectMainframe(const QHostAddress &address, quint16 port)
{
    m_clientSocket->connectToHost(address, port);
}

void SocketConnection::onReadyRead()
{
    // prepare a container to hold the UTF-8 encoded JSON we receive from the socket
    uchar jsonData;
	char data3270; 
	char response[50];
    // create a QDataStream operating on the socket
    QDataStream socketStream(m_clientSocket);
    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    socketStream.setVersion(QDataStream::Qt_5_7);
    // start an infinite loop
    for (;;) {
        // we start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        socketStream.startTransaction();
        // we try to read the JSON data
        socketStream.readRawData(&data3270, 1);
		jsonData = (uchar) data3270;

        if (socketStream.commitTransaction()) {
			switch (telnetState)
			{
				case TELNET_STATE_DATA:
					if (jsonData == IAC) 
					{
						std::cout << "IAC received\n";
						telnetState = TELNET_STATE_IAC;
					} else 
					{
						displayDataStream->addByte(jsonData);
					}
					break;
				case TELNET_STATE_IAC:
					switch (jsonData)
					{
						case IAC: // double IAC means a data byte 0xFF
							displayDataStream->addByte(jsonData);
							telnetState = TELNET_STATE_DATA;
							break;
						case DO:		// Request something, or confirm WILL request
							printf("  DO seen\n"); 
							telnetState = TELNET_STATE_IAC_DO;
							break;
						case DONT: 		// Request to not do something, or reject WILL request
							printf("  DONT seen\n");
							telnetState = TELNET_STATE_IAC_DONT;
							break;
						case WILL:  	// Offer to do something, or confirm DO request
							printf("  WILL seen\n");
							telnetState = TELNET_STATE_IAC_WILL;
							break;
						case WONT: 		// Reject DO request
							printf("  WONT seen\n");
							telnetState = TELNET_STATE_IAC_WONT;
							break;
						case SB:
							printf("  SB seen\n");
							telnetState = TELNET_STATE_IAC_SB;
							break;
						case EOR:
							printf("  EOR seen - yippee!\n");
							emit dataStreamComplete();
							break;
						default:
							printf("IAC Not sure: %02X\n", jsonData);
							break;
					}
					break;
				case TELNET_STATE_IAC_DO:
					switch (jsonData)
					{
						case TELOPT_TTYPE:
						case TELOPT_BINARY:
						case TELOPT_EOR:
							printf("    TTYPE, BINARY or EOR seen\n");
							sprintf(response, "%c%c%c", (char) IAC, (char) WILL, jsonData);
							socketStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							printHex(&response[0]);
							break;
						default:
							sprintf(response, "%c%c%c", (char) IAC, (char) WONT, jsonData);
							socketStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							printHex(&response[0]);
							printf("TTYPE Not sure: %02X\n", jsonData);
							break;
					}
					break;		
				case TELNET_STATE_IAC_DONT:
					printf("IAC DON'T - Not sure: %02X\n", jsonData);
					telnetState = TELNET_STATE_DATA;
					break;
				case TELNET_STATE_IAC_WILL:
					switch (jsonData)
					{
						case TELOPT_BINARY:
						case TELOPT_EOR:
							printf("    BINARY/EOR seen\n");
							sprintf(response, "%c%c%c", (char) IAC, (char) DO, jsonData);
							socketStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							printHex(&response[0]);
							break;
						default:
							sprintf(response, "%c%c%c", (char) IAC, (char) DONT, jsonData);
							socketStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							printHex(&response[0]);
							printf("WILL Not sure: %02X\n", jsonData);
							break;
					}
					break;
				case TELNET_STATE_IAC_WONT:
					printf("IAC WON'T - Not sure: %02X\n", jsonData);
					telnetState = TELNET_STATE_DATA;
					break;
				case TELNET_STATE_IAC_SB:
					switch(jsonData) 
					{
						case IAC:
							telnetState = TELNET_STATE_SB_IAC;
							printf("   IAC in SB seen\n");
							break;
						case SE:
							telnetState = TELNET_STATE_DATA;
							printf("    SE seen\n");
							break;
						case TELOPT_TTYPE:
							telnetState = TELNET_STATE_SB_TTYPE;
							printf("    SB TTYPE seen\n");
							break;
						default:
							printf("IAC SB Not sure: %02.2X\n", jsonData);
					}
					break;
				case TELNET_STATE_SB_TTYPE:
					switch(jsonData) 
					{
						case TELQUAL_SEND:
							telnetState = TELNET_STATE_SB_TTYPE_SEND;
							printf("    SB TTYPE SEND seen\n");
							break;
						default:
							printf("SB TTYPE Not sure: %02.2X\n", jsonData);
					}
					break;
				case TELNET_STATE_SB_TTYPE_SEND:
					switch(jsonData)
					{
						case IAC:
							printf("    SB TTYPE SEND IAC seen\n");
							telnetState = TELNET_STATE_SB_TTYPE_SEND_IAC;
							break;
						default:
							printf("SB TTYPE SEND Not sure: %02.2X\n", jsonData);
					}
					break;
				case TELNET_STATE_SB_TTYPE_SEND_IAC:
					switch(jsonData)
					{
						case SE:
							printf("    SB TTYPE SEND IAC SE seen - good!\n");
							sprintf(response, "%c%c%c%cIBM-3279-2%c%c", (char) IAC, (char) SB, (char) TELOPT_TTYPE, (char) TELQUAL_IS, (char) IAC, (char) SE);
							socketStream.writeRawData(response, 16);
							telnetState = TELNET_STATE_DATA;
							printHex(&response[0]);
							break;
						default:
							printf("SB TTYPE SEND IAC Not sure: %02.2X\n", jsonData);							
							break;
					}
					break;				
				default:
					printf("telnetState Not sure! : %02.2X\n", jsonData);
			}
				
            // we successfully read some data
            // we now need to make sure it's in fact a valid JSON
            //QJsonParseError parseError;
            // we try to create a json document with the data we received
//            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
    //        if (parseError.error == QJsonParseError::NoError) {
                // if the data was indeed valid JSON
      //          if (jsonDoc.isObject()) // and is a JSON object
      //              jsonReceived(jsonDoc.object()); // parse the JSON
      //      }
            // loop and try to read more JSONs if they are available
            fflush(stdout);
        } else {
            // the read failed, the socket goes automatically back to the state it was in before the transaction started
            // we just exit the loop and wait for more data to become available
           break;
       }
    }
}

void SocketConnection::printHex(char *st)
{
	printf("sent: ");
	for(int i=0; *st!=0; i++)
	{
		printf("%02.2X ", (uchar) *st++);
	}
	printf("\n");
	fflush(stdout);
}
