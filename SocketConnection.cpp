#include "SocketConnection.h"

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
    connect(dataSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &SocketConnection::error);
    // Reset the m_loggedIn variable when we disconnec. Since the operation is trivial we use a lambda instead of creating another slot

    incomingData = new Buffer();
    subNeg = new Buffer();
    tn3270e_Mode = false;
}


void SocketConnection::disconnectMainframe()
{
    dataSocket->disconnectFromHost();
}

void SocketConnection::connectMainframe(const QHostAddress &address, quint16 port, ProcessDataStream *d, Terminal *t)
{
    dataSocket->connectToHost(address, port);
    displayDataStream = d;
    term = t;
    connect(displayDataStream, &ProcessDataStream::bufferReady, this, &SocketConnection::sendResponse);
}

void SocketConnection::onReadyRead()
{

    if (incomingData->processing()) {
        printf("SocketConnection : Already processing - cannot process, returning\n");
        fflush(stdout);
       return;
    }

    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);
    //FIXME: What's this used for?
    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    dataStream.setVersion(QDataStream::Qt_5_7);

//    printf("SocketConnection : Buffer allocated %ld\n", incomingData->address());
    fflush(stdout);

//    incomingData->reset();
//    TelnetState oldtelnetState = telnetState;

    // prepare a container to hold the UTF-8 encoded JSON we receive from the socket
    uchar socketByte;
	char data3270; 
	char response[50];
    bool readingSB;

    readingSB = false;
    int byteCount = 0;

    // start an infinite loop
    for (;;) {
        // we start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        dataStream.startTransaction();
        // we try to read the JSON data
        dataStream.readRawData(&data3270, 1);
        socketByte = (uchar) data3270;

        if (dataStream.commitTransaction()) {
//            if (byteCount == 0)
//            {
//                printf("SocketConnection : ");
//            }
//            printf("%2.2X ", socketByte);
            byteCount++;
            if (byteCount>32)
            {
//                printf("\nSocketConnection : ");
                byteCount = 0;
            }
            fflush(stdout);
//            printf("Byte: %02X\n", socketByte);
            switch (telnetState)
			{

				case TELNET_STATE_DATA:
                    if (socketByte == IAC)
					{
                        printf("SocketConnection : IAC seen\n");
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
                            if (readingSB)
                            {
                                subNeg->add(socketByte);
                                telnetState = TELNET_STATE_SB;
                            }
                            else
                            {
                                incomingData->add(socketByte);
                                telnetState = TELNET_STATE_DATA;
                            }
                            printf("SocketConnection : Double 0xFF - stored 0xFF in buffer\n");
							break;
						case DO:		// Request something, or confirm WILL request
                            printf("SocketConnection :   DO seen\n");
							telnetState = TELNET_STATE_IAC_DO;
							break;
						case DONT: 		// Request to not do something, or reject WILL request
                            printf("SocketConnection :   DONT seen\n");
							telnetState = TELNET_STATE_IAC_DONT;
							break;
						case WILL:  	// Offer to do something, or confirm DO request
                            printf("SocketConnection :   WILL seen\n");
							telnetState = TELNET_STATE_IAC_WILL;
							break;
						case WONT: 		// Reject DO request
                            printf("SocketConnection :   WONT seen\n");
							telnetState = TELNET_STATE_IAC_WONT;
							break;
						case SB:
                            printf("SocketConnection :   SB seen\n");
                            telnetState = TELNET_STATE_SB;
                            readingSB = true;
							break;
                        case SE:
                            printf("SocketConnection :   SE seen\n");
                            if (readingSB)
                            {
                               fflush(stdout);
                               processSubNegotiation(subNeg);
                            }
                            else
                            {
                                printf("SocketConnection : IAC SE seen, no SB?\n");
                            }
                            readingSB = false;
                            telnetState = TELNET_STATE_DATA;
                            break;
                        case EOR:
                            incomingData->setProcessing(true);
                            printf("SocketConnection :   EOR\n");
                            fflush(stdout);
                            telnetState = TELNET_STATE_DATA;
                            processBuffer(incomingData);
							break;
						default:
                            printf("SocketConnection : IAC Not sure: %02X\n", socketByte);
							break;
					}
                    fflush(stdout);
                    break;

				case TELNET_STATE_IAC_DO:
                    switch (socketByte)
					{
                        // Note fall-through
                        case TELOPT_TN3270E:
                            tn3270e_Mode = true;
                            printf("SocketConnection : TN3270E switched on\n");
                        case TELOPT_TTYPE:
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            printf("SocketConnection :     TTYPE, BINARY or EOR (%d) seen\n", socketByte);
                            sprintf(response, "%c%c%c", (char) IAC, (char) WILL, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            sprintf(response, "%c%c%c", (char) IAC, (char) WONT, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
                            printf("SocketConnection : TTYPE Not sure: %02X\n", socketByte);
							break;
					}
					break;		

				case TELNET_STATE_IAC_DONT:
                    switch(socketByte)
                    {
                        case TELOPT_TN3270E:
                            tn3270e_Mode = false;
                            printf("SocketConnection : TN3270E switched off\n");
                            break;
                        default:
                            printf("SocketConnection : IAC DON'T - Not sure: %02X\n", socketByte);
                            telnetState = TELNET_STATE_DATA;
                            break;
                    }
                    break;
				case TELNET_STATE_IAC_WILL:
                    switch (socketByte)
					{
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            printf("SocketConnection :     BINARY/EOR seen\n");
                            sprintf(response, "%c%c%c", (char) IAC, (char) DO, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            sprintf(response, "%c%c%c", (char) IAC, (char) DONT, socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
                            printf("SocketConnection : WILL Not sure: %02X\n", socketByte);
							break;
					}
					break;

				case TELNET_STATE_IAC_WONT:
                    printf("SocketConnection : IAC WON'T - Not sure: %02X\n", socketByte);
					telnetState = TELNET_STATE_DATA;
					break;

                case TELNET_STATE_SB:
                    switch(socketByte)
                    {
                        case IAC:
                            telnetState = TELNET_STATE_IAC;
                            printf("SocketConnection : IAC seen in SB processing\n");
                            break;
                        default:
                            subNeg->add(socketByte);
                            break;
                    }
                    break;
				default:
                    printf("SocketConnection : telnetState Not sure! : %2.2X\n", socketByte);
			}
            fflush(stdout);
        } else {
//            incomingData->reset();
//            printf("\nNo more data\n");
            fflush(stdout);
            // the read failed, the socket goes automatically back to the state it was in before the transaction started
            // we just exit the loop and wait for more data to become available
           break;
       }
    }
    if(byteCount != 0)
    {
  //      printf("\n (hopefully we processed these!)");
        byteCount = 0;
    }
    fflush(stdout);
}

void SocketConnection::sendResponse(Buffer *b)
{
    char response[10];
    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);

    if (tn3270e_Mode)
    {
        sprintf(response,"%c%c%c%c%c",(char) TN3270E_DATATYPE_3270_DATA, (char) 0x00, (char) 0x00, (char) 0x00, (char) 0x00);
        dataStream.writeRawData(response, 5);
    }

//    printf("buffer: %d for %d bytes", b->address(), b->size());
    fflush(stdout);
    b->dump(true);
    dataStream.writeRawData(b->address(), b->size());

    response[0] = (char) IAC;
    response[1] = (char) EOR;

    dataStream.writeRawData(response, 2);
}

void SocketConnection::processSubNegotiation(Buffer *buf)
{
    QDataStream dataStream(dataSocket);

    char response[50];

    printf("SocketConnection : -- SubNegotiation --\n");
    buf->dump();

    switch(buf->getByte())
    {
        case TELOPT_TTYPE:
            if (buf->byteEquals(1, TELQUAL_SEND))
            {
                printf("SocketConnection :    SB TTYPE SEND\n");
                fflush(stdout);
                sprintf(response, "%c%c%c%c%s%c%c", (char) IAC, (char) SB, (char) TELOPT_TTYPE, (char) TELQUAL_IS, term->name(), (char) IAC, (char) SE);
                dataStream.writeRawData(response, 6 + strlen(term->name()));
                printf("SocketConnection : (%s)\n",term->name());
                fflush(stdout);
            }
            else
            {
                printf("SocketConnection : Unknown TTYPE subnegotiation: %2.2X\n", buf->getByte(1));
                fflush(stdout);
            }
            break;
        case TELOPT_TN3270E:
            if (buf->byteEquals(1, TN3270E_SEND) && buf->byteEquals(2, TN3270E_DEVICE_TYPE))
            {
                printf("SocketConnection :     SB TN3270E SEND DEVICE_TYPE IAC SE seen - good!\n");
                sprintf(response, "%c%c%c%c%c%s%c%c", (char) IAC, (char) SB, (char) TELOPT_TN3270E, (char) TN3270E_DEVICE_TYPE, (char) TN3270E_REQUEST, term->name(), (char) IAC, (char) SE);
                sprintf(response,"%s%c%c%c%c%c%c%c", response, (char) IAC, (char) SB, (char) TELOPT_TN3270E, (char) TN3270E_FUNCTIONS, (char) TN3270E_REQUEST, (char) IAC, (char) SE);
                int rc = dataStream.writeRawData(response, 14 + strlen(term->name()));
                printf("SocketConnection : (%s) - length %ld: RC=%d. Error=\n", response, 14 + strlen(term->name()), rc);
                fflush(stdout);
                break;
            }
            if (buf->byteEquals(1, TN3270E_DEVICE_TYPE) && buf->byteEquals(2, TN3270E_IS))
            {
                if (buf->compare(3,term->name()) && buf->byteEquals(3+strlen(term->name()), TN3270E_CONNECT))
                {
                    printf("SocketConnection : Received device-name: '");
                    for(int i = 4+strlen(term->name()); i < buf->size(); i++)
                    {
                        printf("%c", buf->getByte(i));
                    }
                    printf("'\n");
                    break;
                }
            }
            if (buf->byteEquals(1, TN3270E_FUNCTIONS) && buf->byteEquals(2, TN3270E_IS))
            {
                printf("SocketConnection : Supported functions: ");
                for(int i = 3; i < buf->size(); i++)
                {
                    printf("%s ", tn3270e_functions_strings[buf->getByte(i)]);
                }
                printf("\n");
                break;
            }
            if (buf->byteEquals(1, TN3270E_FUNCTIONS) && buf->byteEquals(2, TN3270E_REQUEST))
            {
                sprintf(response, "%c%c%c%c%c", (char) IAC, (char) SB, (char) TELOPT_TN3270E, (char) TN3270E_FUNCTIONS, (char) TN3270E_IS);
                printf("SocketConnection : Requested functions:");
                for(int i = 3; i < buf->size(); i++)
                {
                    printf("%s ", tn3270e_functions_strings[buf->getByte(i)]);
                    sprintf(response, "%s%c", response, buf->getByte());
                }
                sprintf(response, "%s%c%c", response, (char) IAC, (char) SE);
                printf("\n");
                int rc = dataStream.writeRawData(response, buf->size()-2 + strlen(response));
                printf("SocketConnection : (%s) - length %ld: RC=%d. Error=\n", response, buf->size()-2 + strlen(response), rc);
                break;
            }
            printf("SocketConnection : Unknown TN3270E request %2.2X\n", buf->getByte(1));
            break;
        default:
            printf("SocketConnection : Unknown Subnegotiation option: %2.2X\n", buf->getByte(0));
            break;
    }
    telnetState = TELNET_STATE_DATA;
    buf->reset();
}

void SocketConnection::processBuffer(Buffer *buf)
{
    if (!tn3270e_Mode)
    {
        emit dataStreamComplete(buf);
        buf->reset();
        return;
    }

    unsigned char dataType = buf->getByte();
    unsigned char requestFlag = buf->nextByte()->getByte();
    unsigned char responseFlag = buf->nextByte()->getByte();
    unsigned char seqNumber = (buf->nextByte()->getByte()<<16) + buf->nextByte()->getByte();

    printf("SocketConnection : TN3270E Header:\nSocketConnection :    Data Type:      %2.2X\nSocketConnection :    Request Flag:   %2.2X\nSocketConnection :    Response Flag:  %2.2X\nSocketConnection :    Sequence Number: %2.2X\n",
                    dataType, requestFlag, responseFlag, seqNumber);

    if (dataType == TN3270E_DATATYPE_3270_DATA)
    {
        emit(dataStreamComplete(buf->nextByte()));
        buf->reset();
    }
}
