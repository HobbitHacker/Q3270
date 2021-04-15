#include "SocketConnection.h"

SocketConnection::SocketConnection(QString termName)
{
    dataSocket = new QTcpSocket(this);
	telnetState = TELNET_STATE_DATA;

    this->termName = termName;
	
    // Forward the connected and disconnected signals
//    connect(dataSocket, &QTcpSocket::connected, this, &SocketConnection::connected);
    connect(dataSocket, &QTcpSocket::disconnected, this, &SocketConnection::closed);
    // connect readyRead() to the slot that will take care of reading the data in
    connect(dataSocket, &QTcpSocket::readyRead, this, &SocketConnection::onReadyRead);
    // Forward the error signal, QOverload is necessary as error() is overloaded, see the Qt docs
//    connect(dataSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &SocketConnection::error);

    tn3270e_Mode = false;
}

SocketConnection::~SocketConnection()
{
    disconnect(dataSocket, &QTcpSocket::disconnected, this, &SocketConnection::closed);
    disconnect(dataSocket, &QTcpSocket::readyRead, this, &SocketConnection::onReadyRead);

    dataSocket->deleteLater();
}

void SocketConnection::closed()
{
    emit disconnected3270();
}

void SocketConnection::disconnectMainframe()
{
    qDebug() << "called disconnect\n";

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
    int byteTot = 0;
    QString charRep;
    QString byteList;
    QString byteNotes;

    // start an infinite loop
    for (;;) {
        // we start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        dataStream.startTransaction();
        // we try to read the incoming data
        dataStream.readRawData(&data3270, 1);
        socketByte = (uchar) data3270;

        if (dataStream.commitTransaction())
        {

            if (byteCount>31)
            {
                qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4").arg(byteTot, 4, 16).arg(byteList.toUpper()).arg(charRep, 32).arg(byteNotes);
                charRep = "";
                byteList = "";
                byteNotes = "";
                byteTot += 32;
                byteCount = 0;
            }

            byteList.append(QString("%1 ").arg(socketByte, 2, 16, QLatin1Char('0')));

            if (isalnum(socketByte))
                charRep.append(socketByte);
            else
                charRep.append(".");

            byteCount++;

            QByteArray response;

            switch (telnetState)
			{

				case TELNET_STATE_DATA:
                    if (socketByte == IAC)
					{
                        byteNotes.append("IAC ");
						telnetState = TELNET_STATE_IAC;
					} else 
					{
                        //printf("Adding Byte: %2.2X - buffer size %d\n", socketByte, incomingData.size());
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
                            byteNotes.append("Double 0xFF ");;
							break;
						case DO:		// Request something, or confirm WILL request
                            byteNotes.append("DO ");
							telnetState = TELNET_STATE_IAC_DO;
							break;
						case DONT: 		// Request to not do something, or reject WILL request
                            byteNotes.append("DONT ");
							telnetState = TELNET_STATE_IAC_DONT;
							break;
						case WILL:  	// Offer to do something, or confirm DO request
                            byteNotes.append("WILL ");
							telnetState = TELNET_STATE_IAC_WILL;
							break;
						case WONT: 		// Reject DO request
                            byteNotes.append("WONT ");
                            telnetState = TELNET_STATE_IAC_WONT;
							break;
						case SB:
                            byteNotes.append("SB ");
                            telnetState = TELNET_STATE_SB;
                            readingSB = true;
							break;
                        case SE:
                            byteNotes.append("SE ");
                            if (readingSB)
                            {
                                if (byteCount>0)
                                {
                                    qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4").arg(byteTot, 4, 16).arg(byteList.toUpper().leftJustified(96)).arg(charRep.leftJustified(32)).arg(byteNotes);
                                    qDebug("");
                                    charRep = "";
                                    byteList = "";
                                    byteNotes = "";
                                    byteTot += 32;
                                    byteCount = 0;
                                }
                               processSubNegotiation();
                            }
                            else
                            {
                                byteNotes.append("- IAC SE, no SB? ");
                            }
                            readingSB = false;
                            telnetState = TELNET_STATE_DATA;
                            break;
                        case EOR:
                            byteNotes.append("EOR ");
                            telnetState = TELNET_STATE_DATA;
                            if (byteCount>0)
                            {
                                qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4").arg(byteTot, 4, 16).arg(byteList.toUpper().leftJustified(96)).arg(charRep.leftJustified(32)).arg(byteNotes);
                                qDebug("");
                                charRep = "";
                                byteList = "";
                                byteNotes = "";
                                byteTot += 32;
                                byteCount = 0;
                            }
                            emit dataStreamComplete(incomingData, tn3270e_Mode);
                            incomingData.clear();
							break;
						default:
                            byteNotes.append(QString("Not sure: %1 ").arg(socketByte, 2, 16));
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
                            byteNotes.append("TN3270E on ");
                        case TELOPT_TTYPE:
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            byteNotes.append(QString("TTYPE, BINARY or EOR (%1) ").arg(socketByte, 2, 16));
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
                            byteNotes.append(QString("Not sure: %1 ").arg(socketByte, 2, 16));
                            break;
					}
					break;		

				case TELNET_STATE_IAC_DONT:
                    switch(socketByte)
                    {
                        case TELOPT_TN3270E:
                            tn3270e_Mode = false;
                            byteNotes.append("TN3270E off ");
                            break;
                        default:
                            byteNotes.append(QString("Not sure: %1 ").arg(socketByte, 2, 16));
                            telnetState = TELNET_STATE_DATA;
                            break;
                    }
                    break;
				case TELNET_STATE_IAC_WILL:
                    switch (socketByte)
					{
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            byteNotes.append("BINARY/EOR ");
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
                            byteNotes.append(QString("Not sure: %1 ").arg(socketByte, 2, 16));
							break;
					}
					break;

				case TELNET_STATE_IAC_WONT:
                    byteNotes.append(QString("Not sure: %1 ").arg(socketByte, 2, 16));
                    telnetState = TELNET_STATE_DATA;
					break;

                case TELNET_STATE_SB:
                    switch(socketByte)
                    {
                        case IAC:
                            telnetState = TELNET_STATE_IAC;
                            byteNotes.append("IAC ");
                            break;
                        default:
                            subNeg.append(socketByte);
                            break;
                    }
                    break;
				default:
                    byteNotes.append(QString("telnetState Not sure! : %1\n").arg(socketByte, 2, 16));
			}
        }
        else
        {
            break;
        }
    }
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

    dump(b, "Sending data");
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

    dump(subNeg, "SubNegotitation");

    switch(subNeg.at(0))
    {
        case TELOPT_TTYPE:
            if (subNeg.at(1) == TELQUAL_SEND)
            {
                qDebug() << "SocketConnection :    SB TTYPE SEND\n";
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
                qDebug() << "SocketConnection : (" << termName << ")\n";
                dump(response, "TELQUAL Response");
                dataStream.writeRawData(response.constData(), response.size());
            }
            else
            {
                qDebug() << QString("SocketConnection : Unknown TTYPE subnegotiation: %1\n").arg((int)subNeg.at(1), 2, 16);
                fflush(stdout);
            }
            break;
        case TELOPT_TN3270E:
            if (subNeg.at(1)  ==  TN3270E_SEND && subNeg.at(2) ==  TN3270E_DEVICE_TYPE)
            {
                qDebug() << "SocketConnection :     SB TN3270E SEND DEVICE_TYPE IAC SE seen\n";

                response.append((uchar) IAC);
                response.append((uchar) SB);
                response.append((uchar) TELOPT_TN3270E);
                response.append((uchar) TN3270E_DEVICE_TYPE);
                response.append((uchar) TN3270E_REQUEST);
                response.append(termName.toLatin1().data(), strlen(termName.toLatin1().data()));
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

                break;
            }
            if (subNeg.at(1) == TN3270E_DEVICE_TYPE && subNeg.at(2) == TN3270E_IS)
            {
                if (subNeg.mid(3).compare(termName.toLatin1().data()) && subNeg.at(3 + termName.length()) == TN3270E_CONNECT)
                {
                    qDebug() << "SocketConnection : Received device-name: '";
                    for(int i = 4+strlen(termName.toLatin1().data()); i < subNeg.size(); i++)
                    {
                        qDebug() << subNeg.at(i);
                    }
                    qDebug() << "'\n";
                    break;
                }
            }
            if (subNeg.at(1) == TN3270E_FUNCTIONS && subNeg.at(2) == TN3270E_IS)
            {
                qDebug() << "SocketConnection : Supported functions: ";
                for(int i = 3; i < subNeg.size(); i++)
                {
                    qDebug() << tn3270e_functions_strings[(int) subNeg.at(i)];
                }
                qDebug() << "\n";
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
                qDebug() << "SocketConnection : Requested functions:";
                for(int i = 3; i < subNeg.size(); i++)
                {
                    qDebug() << tn3270e_functions_strings[(int) subNeg.at(i)];
                    response.append(subNeg.at(i));
                }

                response.append((uchar) IAC);
                response.append((uchar) SE);

                qDebug() << "\n";

                dump(response, "TN3270E Functions Response");

                dataStream.writeRawData(response, response.size());
                break;
            }
            qDebug() << QString("SocketConnection : Unknown TN3270E request %1\n").arg((int)subNeg.at(1), 2, 16);
            break;
        default:
            qDebug() << QString("SocketConnection : Unknown Subnegotiation option: %1\n").arg((int)subNeg.at(0), 2, 16);
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

    qDebug() << title << " Start -------------------------------------------\n";

    for (int i = 0; i < a.size(); i++)
    {
        if (w == 0)
        {
            qDebug() << QString("%1").arg(i, 4, 16);
            bytes = "";
        }

        qDebug() << QString("%1").arg((int)a.at(i), 2, 16);
        if (QChar(a.at(i)).isPrint())
            bytes.append(a.at(i));
        else
            bytes.append(".");
        if (++w == 32) {
            w = 0;
            qDebug() << "| " << bytes << " |\n";
        }
    }
    if (w != 0) {
        int spaces = (32 - w) * 3;
        qDebug() << QString("%1 | %2 |").arg(" ", spaces).arg(bytes);
    }
    qDebug() << "\n" << title << " End   -------------------------------------------\n";

}
