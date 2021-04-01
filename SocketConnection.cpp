#include "SocketConnection.h"

SocketConnection::SocketConnection(QString termName)
{
    dataSocket = new QTcpSocket(this);
	telnetState = TELNET_STATE_DATA;

    this->termName = termName;
	
    // Forward the connected and disconnected signals
//    connect(dataSocket, &QTcpSocket::connected, this, &SocketConnection::connected);
//    connect(dataSocket, &QTcpSocket::disconnected, this, &SocketConnection::closed);
    // connect readyRead() to the slot that will take care of reading the data in
    connect(dataSocket, &QTcpSocket::readyRead, this, &SocketConnection::onReadyRead);
    // Forward the error signal, QOverload is necessary as error() is overloaded, see the Qt docs
//    connect(dataSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &SocketConnection::error);

    tn3270e_Mode = false;
}

void SocketConnection::closed()
{
    emit disconnected3270();
}

void SocketConnection::disconnectMainframe()
{
    printf("called disconnect\n");
    fflush(stdout);
    //TODO: This is called twice when disconnecting
    disconnect(displayDataStream, &ProcessDataStream::bufferReady, this, &SocketConnection::sendResponse);
//    disconnect(dataSocket, &QTcpSocket::connected, this, &SocketConnection::connected);
//    disconnect(dataSocket, &QTcpSocket::disconnected, this, &SocketConnection::disconnected);
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


/*    if (incomingData->processing()) {
        printf("SocketConnection : Already processing - cannot process, returning\n");
        fflush(stdout);
       return;
    }
*/
    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);
    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    dataStream.setVersion(QDataStream::Qt_5_12);

    uchar socketByte;

    char data3270;
    bool readingSB;

    readingSB = false;
    int byteCount = 0;
    QString charRep;

    printf("\n\nSocketConnection : Reset loop\n\n");
    printf("SocketConnection : ");

    // start an infinite loop
    for (;;) {
        // we start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        dataStream.startTransaction();
        // we try to read the incoming data
        dataStream.readRawData(&data3270, 1);
        socketByte = (uchar) data3270;

        if (dataStream.commitTransaction()) {
/*            if (byteCount == 0)
            {
                printf("SocketConnection : ");
                charRep = "";
            }*/
            printf("%2.2X ", socketByte);
            if (isalnum(socketByte))
                charRep = charRep + socketByte;
            else
                charRep = charRep + ".";
            byteCount++;
            if (byteCount>32)
            {
                printf("| %32s |\nSocketConnection : ", charRep.toLatin1().data());
                charRep = "";
                byteCount = 0;
            }
            fflush(stdout);
//            printf("Byte: %02X\n", socketByte);
            QByteArray response;
            switch (telnetState)
			{

				case TELNET_STATE_DATA:
                    if (socketByte == IAC)
					{
                        printf("SocketConnection : IAC seen\n");
						telnetState = TELNET_STATE_IAC;
					} else 
					{
                        printf("Adding Byte: %2.2X - buffer size %d\n", socketByte, incomingData.size());
                        incomingData.append(socketByte);
					}
					break;

				case TELNET_STATE_IAC:
                    switch (socketByte)
					{
						case IAC: // double IAC means a data byte 0xFF
                            if (readingSB)
                            {
                                // socketByte is 0xFF here, so we don't double up the incoming buffer
                                subNeg.append(socketByte);
                                telnetState = TELNET_STATE_SB;
                            }
                            else
                            {
                                // socketByte is 0xFF here, so we don't double up the incoming buffer
                                incomingData.append(socketByte);
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
//                            incomingData->setProcessing(true);
                            printf("SocketConnection :   EOR\n");
                            fflush(stdout);
                            telnetState = TELNET_STATE_DATA;
                            emit dataStreamComplete(incomingData, tn3270e_Mode);
                            incomingData.clear();
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
                            printf("SocketConnection :     TTYPE, BINARY or EOR (%2.2X) seen\n", socketByte);
                            response.append((uchar) IAC);
                            response.append((uchar) WILL);
                            response.append(socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            response.append((uchar) IAC);
                            response.append((uchar) WONT);
                            response.append(socketByte);
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
                            response.append((uchar) IAC);
                            response.append((uchar) DO);
                            response.append(socketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            response.append((uchar) IAC);
                            response.append((uchar) DONT);
                            response.append(socketByte);
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
                            subNeg.append(socketByte);
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
            if (byteCount>32)
            {
                printf("| %32s |\nSocketConnection : ", charRep.toLatin1().data());
            }
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

void SocketConnection::sendResponse(QByteArray &b)
{
    QByteArray response;
    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);

    if (tn3270e_Mode)
    {
        response.append((uchar) TN3270E_DATATYPE_3270_DATA);
        response.append((uchar) 0x00);
        response.append((uchar) 0x00);
        response.append((uchar) 0x00);
        response.append((uchar) 0x00);
        dump(response, "TN3270E Response");
        dataStream.writeRawData(response, 5);
    }

//    printf("buffer: %d for %d bytes", b->address(), b->size());
    fflush(stdout);
//    b->dump(true);
    dataStream.writeRawData(b.constData(), b.size());

    response.clear();
    response.append((uchar) IAC);
    response.append((uchar) EOR);
    dump(response, "EOR Response");

    dataStream.writeRawData(response, 2);
}

void SocketConnection::processSubNegotiation()
{
    QDataStream dataStream(dataSocket);
    QByteArray response;

    printf("SocketConnection : -- SubNegotiation --\n");
//    subNeg->dump();

    dump(subNeg, "SubNegotitation");

    switch(subNeg.at(0))
    {
        case TELOPT_TTYPE:
            if (subNeg.at(1) == TELQUAL_SEND)
            {
                printf("SocketConnection :    SB TTYPE SEND\n");
                fflush(stdout);
                response.append((uchar) IAC);
                response.append((uchar) SB);
                response.append((uchar) TELOPT_TTYPE);
                response.append((uchar) TELQUAL_IS);
                response.append(termName.toLatin1().data(), strlen(termName.toLatin1().data()));

                // Pass LU name if one was requested
                if (luName.compare(""))
                {
                    response.append('@');
                    response.append(luName.toLatin1().data(), strlen(luName.toLatin1().data()));
                }

                response.append((uchar) IAC);
                response.append((uchar) SE);
                printf("SocketConnection : (%s)\n",termName.toLatin1().data());
                fflush(stdout);
                dump(response, "TELQUAL Response");
                dataStream.writeRawData(response.constData(), response.size());
            }
            else
            {
                printf("SocketConnection : Unknown TTYPE subnegotiation: %2.2X\n", subNeg.at(1));
                fflush(stdout);
            }
            break;
        case TELOPT_TN3270E:
            if (subNeg.at(1)  ==  TN3270E_SEND && subNeg.at(2) ==  TN3270E_DEVICE_TYPE)
            {
                printf("SocketConnection :     SB TN3270E SEND DEVICE_TYPE IAC SE seen - good!\n");

                response.append((uchar) IAC);
                response.append((uchar) SB);
                response.append((uchar) TELOPT_TN3270E);
                response.append((uchar) TN3270E_DEVICE_TYPE);
                response.append((uchar) TN3270E_REQUEST);
                response.append(termName.constData()->toLatin1(), strlen(termName.toLatin1().data()));
                response.append((uchar) IAC);
                response.append((uchar) SE);

                response.append((uchar) IAC);
                response.append((uchar) SB);
                response.append((uchar) TELOPT_TN3270E);
                response.append((uchar) TN3270E_FUNCTIONS);
                response.append((uchar) TN3270E_REQUEST);
                response.append((uchar) IAC);
                response.append((uchar) SE);

                dump(response, "TELOPT Response");

                dataStream.writeRawData(response, response.size());

                fflush(stdout);
                break;
            }
            if (subNeg.at(1) == TN3270E_DEVICE_TYPE && subNeg.at(2) == TN3270E_IS)
            {
                if (subNeg.mid(3).compare(termName.toLatin1().data()) && subNeg.at(3 + termName.length()) == TN3270E_CONNECT)
                {
                    printf("SocketConnection : Received device-name: '");
                    for(int i = 4+strlen(termName.toLatin1().data()); i < subNeg.size(); i++)
                    {
                        printf("%c", subNeg.at(i));
                    }
                    printf("'\n");
                    break;
                }
            }
            if (subNeg.at(1) == TN3270E_FUNCTIONS && subNeg.at(2) == TN3270E_IS)
            {
                printf("SocketConnection : Supported functions: ");
                for(int i = 3; i < subNeg.size(); i++)
                {
                    printf("%s ", tn3270e_functions_strings[(int) subNeg.at(i)]);
                }
                printf("\n");
                break;
            }
            if (subNeg.at(1) == TN3270E_FUNCTIONS && subNeg.at(2) == TN3270E_REQUEST)
            {
                response.append((uchar) IAC);
                response.append((uchar) SB);
                response.append((uchar) TELOPT_TN3270E);
                response.append((uchar) TN3270E_FUNCTIONS);
                response.append((uchar) TN3270E_IS);

                //TODO: Unsupported functions
                printf("SocketConnection : Requested functions:");
                for(int i = 3; i < subNeg.size(); i++)
                {
                    printf("%s ", tn3270e_functions_strings[(int) subNeg.at(i)]);
                    response.append(subNeg.at(i));
                }

                response.append((uchar) IAC);
                response.append((uchar) SE);

                printf("\n");

                dump(response, "TN3270E Functions Response");

                dataStream.writeRawData(response, response.size());
                break;
            }
            printf("SocketConnection : Unknown TN3270E request %2.2X\n", subNeg.at(1));
            break;
        default:
            printf("SocketConnection : Unknown Subnegotiation option: %2.2X\n", subNeg.at(0));
            break;
    }
    telnetState = TELNET_STATE_DATA;
    //FIXME Is reset() an error here? Will data appear asynchronously in the buffer?
    subNeg.clear();
}

void SocketConnection::dump(QByteArray &a, QString title)
{
    int w = 0;
    QString bytes;

    printf("%s Start -------------------------------------------\n", title.toLatin1().data());

    for (int i = 0; i < a.size(); i++)
    {
        if (w == 0)
            printf("%4.4X ", i);

        printf("%2.2X ", (uchar) a.at(i));
        if (QChar(a.at(i)).isPrint())
            bytes = bytes + a.at(i);
        else
            bytes = bytes + ".";
        if (++w == 32) {
            w = 0;
            printf ("| %-32.32s |\n", bytes.toLatin1().data());
        }
    }
    if (w != 0) {
//        for (int i = w; i < 32; i++)
//            printf("   ");
        char format[50];
        sprintf(&format[0], "%%%d.%ds| %%-32.32s |", (32 - w) * 3, (32 - w) * 3 );
        printf (format, " ", bytes.toLatin1().data());
    }
    printf("\n%s End   -------------------------------------------\n", title.toLatin1().data());

    fflush(stdout);
}
