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
    , dataSocket(new QTcpSocket(this))
{
	telnetState = TELNET_STATE_DATA;
	
    // Forward the connected and disconnected signals
    connect(dataSocket, &QTcpSocket::connected, this, &SocketConnection::connected);
    connect(dataSocket, &QTcpSocket::disconnected, this, &SocketConnection::disconnected);
    // connect readyRead() to the slot that will take care of reading the data in
    connect(dataSocket, &QTcpSocket::readyRead, this, &SocketConnection::onReadyRead);
    // Forward the error signal, QOverload is necessary as error() is overloaded, see the Qt docs
    connect(dataSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &SocketConnection::error);
    // Reset the m_loggedIn variable when we disconnec. Since the operation is trivial we use a lambda instead of creating another slot

    incomingData = new Buffer();
}


void SocketConnection::disconnectMainframe()
{
    dataSocket->disconnectFromHost();
}

void SocketConnection::connectMainframe(const QHostAddress &address, quint16 port, DisplayDataStream *d)
{
    dataSocket->connectToHost(address, port);
    displayDataStream = d;
    connect(displayDataStream, &DisplayDataStream::bufferReady, this, &SocketConnection::sendResponse);
}

void SocketConnection::onReadyRead()
{

    if (incomingData->processing())
       return;

    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);
    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    dataStream.setVersion(QDataStream::Qt_5_7);

    printf("Buffer allocated %d\n", incomingData->address());
    fflush(stdout);

    incomingData->reset();

    // prepare a container to hold the UTF-8 encoded JSON we receive from the socket
    uchar socketByte;
	char data3270; 
	char response[50];
    // start an infinite loop
    for (;;) {
        // we start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        dataStream.startTransaction();
        // we try to read the JSON data
        dataStream.readRawData(&data3270, 1);
        socketByte = (uchar) data3270;

        if (dataStream.commitTransaction()) {
//            printf("Byte: %02X\n", socketByte);
            switch (telnetState)
			{
				case TELNET_STATE_DATA:
                    if (socketByte == IAC)
					{
						std::cout << "IAC received\n";
						telnetState = TELNET_STATE_IAC;
					} else 
					{
                        incomingData->add(socketByte);
					}
					break;
				case TELNET_STATE_IAC:
                    switch (socketByte)
					{
						case IAC: // double IAC means a data byte 0xFF
                            std::cout << "Double 0xFF received - stored 0xFF\n";
                            incomingData->add(socketByte);
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
                            incomingData->setProcessing(true);
							printf("  EOR seen - yippee!\n");
                            telnetState = TELNET_STATE_DATA;
                            emit dataStreamComplete(incomingData);
							break;
						default:
                            printf("IAC Not sure: %02X\n", socketByte);
							break;
					}
					break;
				case TELNET_STATE_IAC_DO:
                    switch (socketByte)
					{
						case TELOPT_TTYPE:
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            printf("    TTYPE, BINARY or EOR (%d) seen\n", socketByte);
                            sprintf(response, "%c%c%c", (char) IAC, (char) WILL, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            sprintf(response, "%c%c%c", (char) IAC, (char) WONT, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
                            printf("TTYPE Not sure: %02X\n", socketByte);
							break;
					}
					break;		
				case TELNET_STATE_IAC_DONT:
                    printf("IAC DON'T - Not sure: %02X\n", socketByte);
					telnetState = TELNET_STATE_DATA;
					break;
				case TELNET_STATE_IAC_WILL:
                    switch (socketByte)
					{
						case TELOPT_BINARY:
						case TELOPT_EOR:
							printf("    BINARY/EOR seen\n");
                            sprintf(response, "%c%c%c", (char) IAC, (char) DO, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            sprintf(response, "%c%c%c", (char) IAC, (char) DONT, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
                            printf("WILL Not sure: %02X\n", socketByte);
							break;
					}
					break;
				case TELNET_STATE_IAC_WONT:
                    printf("IAC WON'T - Not sure: %02X\n", socketByte);
					telnetState = TELNET_STATE_DATA;
					break;
				case TELNET_STATE_IAC_SB:
                    switch(socketByte)
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
                            printf("IAC SB Not sure: %02.2X\n", socketByte);
					}
					break;
				case TELNET_STATE_SB_TTYPE:
                    switch(socketByte)
					{
						case TELQUAL_SEND:
							telnetState = TELNET_STATE_SB_TTYPE_SEND;
							printf("    SB TTYPE SEND seen\n");
							break;
						default:
                            printf("SB TTYPE Not sure: %02.2X\n", socketByte);
					}
					break;
				case TELNET_STATE_SB_TTYPE_SEND:
                    switch(socketByte)
					{
						case IAC:
							printf("    SB TTYPE SEND IAC seen\n");
							telnetState = TELNET_STATE_SB_TTYPE_SEND_IAC;
							break;
						default:
                            printf("SB TTYPE SEND Not sure: %02.2X\n", socketByte);
					}
					break;
				case TELNET_STATE_SB_TTYPE_SEND_IAC:
                    switch(socketByte)
					{
						case SE:
							printf("    SB TTYPE SEND IAC SE seen - good!\n");
                            sprintf(response, "%c%c%c%cIBM-3279-2-E%c%c", (char) IAC, (char) SB, (char) TELOPT_TTYPE, (char) TELQUAL_IS, (char) IAC, (char) SE);
                            dataStream.writeRawData(response, 18);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            printf("SB TTYPE SEND IAC Not sure: %02.2X\n", socketByte);
							break;
					}
					break;				
				default:
                    printf("telnetState Not sure! : %02.2X\n", socketByte);
			}
            fflush(stdout);
        } else {
            incomingData->reset();
            // the read failed, the socket goes automatically back to the state it was in before the transaction started
            // we just exit the loop and wait for more data to become available
           break;
       }
    }
    printf("Loop done\n");
    fflush(stdout);
}

void SocketConnection::sendResponse(Buffer *b)
{
    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);

    printf("buffer: %d for %d bytes", b->address(), b->size());
    fflush(stdout);
    dataStream.writeRawData(b->address(), b->size());

    char response[2];

    response[0] = IAC;
    response[1] = EOR;

    dataStream.writeRawData(response, 2);

}
