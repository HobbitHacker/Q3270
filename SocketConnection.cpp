#include "SocketConnection.h"

SocketConnection::SocketConnection(QString termName)
{
    dataSocket = new QTcpSocket(this);
	telnetState = TELNET_STATE_DATA;

    this->termName = termName;
	
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
    disconnect(displayDataStream, &ProcessDataStream::bufferReady, this, &SocketConnection::sendResponse);
    disconnect(dataSocket, &QTcpSocket::connected, this, &SocketConnection::connected);
    disconnect(dataSocket, &QTcpSocket::disconnected, this, &SocketConnection::disconnected);
    disconnect(dataSocket, &QTcpSocket::readyRead, this, &SocketConnection::onReadyRead);

    dataSocket->disconnectFromHost();
}

void SocketConnection::connectMainframe(const QHostAddress &address, quint16 port, QString luName, ProcessDataStream *d)
{
    dataSocket->connectToHost(address, port);
    displayDataStream = d;
    connect(displayDataStream, &ProcessDataStream::bufferReady, this, &SocketConnection::sendResponse);
    this->luName = luName;
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
    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    dataStream.setVersion(QDataStream::Qt_5_15);

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
        // we try to read the incoming data
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
                               processSubNegotiation();
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
                            processBuffer();
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
/*    if(byteCount != 0)
    {
  //      printf("\n (hopefully we processed these!)");
        byteCount = 0;
    }*/
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

void SocketConnection::processSubNegotiation()
{
    QDataStream dataStream(dataSocket);

    Buffer *response = new Buffer();

    printf("SocketConnection : -- SubNegotiation --\n");
    subNeg->dump();

    switch(subNeg->getByte())
    {
        case TELOPT_TTYPE:
            if (subNeg->byteEquals(1, TELQUAL_SEND))
            {
                printf("SocketConnection :    SB TTYPE SEND\n");
                fflush(stdout);
                response->add(IAC);
                response->add(SB);
                response->add(TELOPT_TTYPE);
                response->add(TELQUAL_IS);
                response->addBlock((unsigned char *)termName.toLatin1().data(), strlen(termName.toLatin1().data()));

                // Pass LU name if one was requested
                if (luName.compare(""))
                {
                    response->add('@');
                    response->addBlock((unsigned char *)luName.toLatin1().data(), strlen(luName.toLatin1().data()));
                }

                response->add(IAC);
                response->add(SE);
                printf("SocketConnection : (%s)\n",termName.toLatin1().data());
                fflush(stdout);
                dataStream.writeRawData(response->address(), response->size());
            }
            else
            {
                printf("SocketConnection : Unknown TTYPE subnegotiation: %2.2X\n", subNeg->getByte(1));
                fflush(stdout);
            }
            break;
        case TELOPT_TN3270E:
            if (subNeg->byteEquals(1, TN3270E_SEND) && subNeg->byteEquals(2, TN3270E_DEVICE_TYPE))
            {
                printf("SocketConnection :     SB TN3270E SEND DEVICE_TYPE IAC SE seen - good!\n");

                response->add(IAC);
                response->add(SB);
                response->add(TELOPT_TN3270E);
                response->add(TN3270E_DEVICE_TYPE);
                response->add(TN3270E_REQUEST);
                response->addBlock((unsigned char *)termName.toLatin1().data(), strlen(termName.toLatin1().data()));
                response->add(IAC);
                response->add(SE);

                response->add(IAC);
                response->add(SB);
                response->add(TELOPT_TN3270E);
                response->add(TN3270E_FUNCTIONS);
                response->add(TN3270E_REQUEST);
                response->add(IAC);
                response->add(SE);
                dataStream.writeRawData(response->address(), response->size());

                fflush(stdout);
                break;
            }
            if (subNeg->byteEquals(1, TN3270E_DEVICE_TYPE) && subNeg->byteEquals(2, TN3270E_IS))
            {
                if (subNeg->compare(3,termName.toLatin1().data()) && subNeg->byteEquals(3+strlen(termName.toLatin1().data()), TN3270E_CONNECT))
                {
                    printf("SocketConnection : Received device-name: '");
                    for(int i = 4+strlen(termName.toLatin1().data()); i < subNeg->size(); i++)
                    {
                        printf("%c", subNeg->getByte(i));
                    }
                    printf("'\n");
                    break;
                }
            }
            if (subNeg->byteEquals(1, TN3270E_FUNCTIONS) && subNeg->byteEquals(2, TN3270E_IS))
            {
                printf("SocketConnection : Supported functions: ");
                for(int i = 3; i < subNeg->size(); i++)
                {
                    printf("%s ", tn3270e_functions_strings[subNeg->getByte(i)]);
                }
                printf("\n");
                break;
            }
            if (subNeg->byteEquals(1, TN3270E_FUNCTIONS) && subNeg->byteEquals(2, TN3270E_REQUEST))
            {
                response->add(IAC);
                response->add(SB);
                response->add(TELOPT_TN3270E);
                response->add(TN3270E_FUNCTIONS);
                response->add(TN3270E_IS);

                printf("SocketConnection : Requested functions:");
                for(int i = 3; i < subNeg->size(); i++)
                {
                    printf("%s ", tn3270e_functions_strings[subNeg->getByte(i)]);
                    response->add(subNeg->getByte());
                }

                response->add(IAC);
                response->add(SE);

                printf("\n");
                dataStream.writeRawData(response->address(), response->size());
                break;
            }
            printf("SocketConnection : Unknown TN3270E request %2.2X\n", subNeg->getByte(1));
            break;
        default:
            printf("SocketConnection : Unknown Subnegotiation option: %2.2X\n", subNeg->getByte(0));
            break;
    }
    telnetState = TELNET_STATE_DATA;
    //FIXME Is reset() an error here? Will data appear asynchronously in the buffer?
    subNeg->reset();
}

void SocketConnection::processBuffer()
{
    if (!tn3270e_Mode)
    {
        emit dataStreamComplete(incomingData);
        incomingData->reset();
        return;
    }

    unsigned char dataType = incomingData->getByte();
    unsigned char requestFlag = incomingData->nextByte()->getByte();
    unsigned char responseFlag = incomingData->nextByte()->getByte();
    unsigned char seqNumber = (incomingData->nextByte()->getByte()<<16) + incomingData->nextByte()->getByte();

    printf("SocketConnection : TN3270E Header:\nSocketConnection :    Data Type:      %2.2X\nSocketConnection :    Request Flag:   %2.2X\nSocketConnection :    Response Flag:  %2.2X\nSocketConnection :    Sequence Number: %2.2X\n",
                    dataType, requestFlag, responseFlag, seqNumber);

    if (dataType == TN3270E_DATATYPE_3270_DATA)
    {
        emit dataStreamComplete(incomingData->nextByte());
        incomingData->reset();
    }
}
