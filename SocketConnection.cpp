#include "SocketConnection.h"

SocketConnection::SocketConnection(QString termName)
{
    dataSocket = new QTcpSocket(this);
	telnetState = TELNET_STATE_DATA;

    this->termName = termName;
	
    // Forward the connected and disconnected signals
//    connect(dataSocket, &QTcpSocket::connected, this, &SocketConnection::connected);
    connect(dataSocket, &QTcpSocket::disconnected, this, &SocketConnection::closed);

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

    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);

    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    dataStream.setVersion(QDataStream::Qt_5_12);

    // Incoming byte - unsigned
    uchar unsignedSocketByte;

    char socketByte;
    bool subNegotiationProcessing = false;

    // Debugging - formatted data dump variables
    int byteCount = 0;
    int byteTot   = 0;

    QString charRep;
    QString byteList;
    QString byteNotes;

    // start an infinite loop
    for (;;)
    {
        // Start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        dataStream.startTransaction();

        // Read incoming data, and convert to unsigned
        dataStream.readRawData(&socketByte, 1);
        unsignedSocketByte = (uchar) socketByte;

        if (dataStream.commitTransaction())
        {

            // Debugging - if we've reached 32 bytes, pretty print them.
            if (byteCount>31)
            {
                qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4").arg(byteTot, 4, 16).arg(byteList.toUpper()).arg(charRep, 32).arg(byteNotes);
                charRep = "";
                byteList = "";
                byteNotes = "";
                byteTot += 32;
                byteCount = 0;
            }

            // Debugging - add this byte to the hexdump
            byteList.append(QString("%1 ").arg(unsignedSocketByte, 2, 16, QLatin1Char('0')));

            // Debugging - add this byte as a character if it's alphanumeric
            if (isalnum(unsignedSocketByte))
                charRep.append(unsignedSocketByte);
            else
                charRep.append(".");

            byteCount++;

            QByteArray response;

            qDebug();

            switch (telnetState)
			{

				case TELNET_STATE_DATA:
                    if (unsignedSocketByte == IAC)
					{
                        byteNotes.append("IAC ");
						telnetState = TELNET_STATE_IAC;
					} else 
					{
                        incomingData.append(unsignedSocketByte);
					}
					break;

				case TELNET_STATE_IAC:
                    switch (unsignedSocketByte)
					{
                        case IAC: // double IAC (0xFF) means a single data byte 0xFF
                            if (subNegotiationProcessing)
                            {
                                // socketByte is 0xFF here, so we don't double up the incoming buffer
                                subNegotiationBuffer.append(unsignedSocketByte);
                                telnetState = TELNET_STATE_SB;
                            }
                            else
                            {
                                // socketByte is 0xFF here, so we don't double up the incoming buffer
                                incomingData.append(unsignedSocketByte);
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
                            subNegotiationProcessing = true;
							break;
                        case SE:
                            byteNotes.append("SE ");
                            if (subNegotiationProcessing)
                            {
                                // Debugging - if there are bytes we haven't printed, print them.
                                if (byteCount>0)
                                {
                                    qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4").arg(byteTot, 4, 16).arg(byteList.toUpper().leftJustified(96)).arg(charRep.leftJustified(32)).arg(byteNotes);
                                    charRep = "";
                                    byteList = "";
                                    byteNotes = "";
                                    byteTot += 32;
                                    byteCount = 0;
                                }
                                qDebug() << "";
                                processSubNegotiation();
                            }
                            else
                            {
                                byteNotes.append("- IAC SE, no SB? ");
                            }
                            subNegotiationProcessing = false;
                            telnetState = TELNET_STATE_DATA;
                            break;
                        case EOR:
                            byteNotes.append("EOR ");
                            telnetState = TELNET_STATE_DATA;
                            if (byteCount>0)
                            {
                                // Debugging - if there are bytes we haven't printed, print them.
                                qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4").arg(byteTot, 4, 16).arg(byteList.toUpper().leftJustified(96)).arg(charRep.leftJustified(32)).arg(byteNotes);
                                charRep = "";
                                byteList = "";
                                byteNotes = "";
                                byteTot += 32;
                                byteCount = 0;
                            }
                            qDebug() << "";
                            emit dataStreamComplete(incomingData, tn3270e_Mode);
                            incomingData.clear();
							break;
						default:
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
							break;
					}
                    break;

				case TELNET_STATE_IAC_DO:
                    switch (unsignedSocketByte)
					{
                        // Note fall-through
                        case TELOPT_TN3270E:
                            tn3270e_Mode = true;
                            byteNotes.append("TN3270E on ");
                        case TELOPT_TTYPE:
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            byteNotes.append(QString("TTYPE, BINARY or EOR (%1) ").arg(unsignedSocketByte, 2, 16));
                            response.append((uchar) IAC);
                            response.append((uchar) WILL);
                            response.append(unsignedSocketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            response.append((uchar) IAC);
                            response.append((uchar) WONT);
                            response.append(unsignedSocketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
                            break;
					}
					break;		

				case TELNET_STATE_IAC_DONT:
                    switch(unsignedSocketByte)
                    {
                        case TELOPT_TN3270E:
                            tn3270e_Mode = false;
                            byteNotes.append("TN3270E off ");
                            break;
                        default:
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
                            telnetState = TELNET_STATE_DATA;
                            break;
                    }
                    break;
				case TELNET_STATE_IAC_WILL:
                    switch (unsignedSocketByte)
					{
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            byteNotes.append("BINARY/EOR ");
                            response.append((uchar) IAC);
                            response.append((uchar) DO);
                            response.append(unsignedSocketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
							break;
						default:
                            response.append((uchar) IAC);
                            response.append((uchar) DONT);
                            response.append(unsignedSocketByte);
                            dataStream.writeRawData(response, 3);
							telnetState = TELNET_STATE_DATA;
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
							break;
					}
					break;

				case TELNET_STATE_IAC_WONT:
                    byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
                    telnetState = TELNET_STATE_DATA;
					break;

                case TELNET_STATE_SB:
                    switch(unsignedSocketByte)
                    {
                        case IAC:
                            telnetState = TELNET_STATE_IAC;
                            byteNotes.append("IAC ");
                            break;
                        default:
                            subNegotiationBuffer.append(unsignedSocketByte);
                            break;
                    }
                    break;
				default:
                    byteNotes.append(QString("telnetState Not sure! : %1\n").arg(unsignedSocketByte, 2, 16));
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
        dump(response, "Send TN3270E Response");
        dataStream.writeRawData(response, 5);
    }

    dump(b, "Sending data");
    dataStream.writeRawData(b.constData(), b.size());

    response.clear();
    response.append((uchar) IAC);
    response.append((uchar) EOR);
    dump(response, "Sending EOR Response");

    dataStream.writeRawData(response, 2);
}

void SocketConnection::processSubNegotiation()
{
    QDataStream dataStream(dataSocket);
    QByteArray response;

    switch(subNegotiationBuffer.at(0))
    {
        case TELOPT_TTYPE:
            if (subNegotiationBuffer.at(1) == TELQUAL_SEND)
            {
                dump(subNegotiationBuffer, "SubNegotitation - TTYPE SEND");

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

                dump(response, "SubNegotiation - TTYPE SEND Response");

                dataStream.writeRawData(response.constData(), response.size());
            }
            else
            {
                dump(subNegotiationBuffer, "SubNegotiation - TTYPE unknown");
            }
            break;
        case TELOPT_TN3270E:
            if (subNegotiationBuffer.at(1)  ==  TN3270E_SEND && subNegotiationBuffer.at(2) ==  TN3270E_DEVICE_TYPE)
            {
                dump(subNegotiationBuffer, "SubNegotitation - TN3270E SEND TTYPE");

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

                dump(response, "SubNegotiation - TN3270E SEND TTYPE Response");

                dataStream.writeRawData(response, response.size());

                break;
            }
            if (subNegotiationBuffer.at(1) == TN3270E_DEVICE_TYPE && subNegotiationBuffer.at(2) == TN3270E_IS)
            {
                dump(subNegotiationBuffer, "SubNegotitation - TTYPE IS");

/*                if (subNeg.mid(3).compare(termName.toLatin1().data()) && subNeg.at(3 + termName.length()) == TN3270E_CONNECT)
                {
                    qDebug() << "SocketConnection : Received device-name: '";
                    for(int i = 4+strlen(termName.toLatin1().data()); i < subNeg.size(); i++)
                    {
                        qDebug() << subNeg.at(i);
                    }
                    qDebug() << "'\n";
                    break;
                }
*/
            }
            if (subNegotiationBuffer.at(1) == TN3270E_FUNCTIONS && subNegotiationBuffer.at(2) == TN3270E_IS)
            {
                dump(subNegotiationBuffer, "SubNegotitation - TN3270E FNCTIONS IS");
/*                qDebug() << "SocketConnection : Supported functions: ";
                for(int i = 3; i < subNeg.size(); i++)
                {
                    qDebug() << tn3270e_functions_strings[(int) subNeg.at(i)];
                }
                qDebug() << "\n";
*/                break;
            }
            if (subNegotiationBuffer.at(1) == TN3270E_FUNCTIONS && subNegotiationBuffer.at(2) == TN3270E_REQUEST)
            {
                dump(subNegotiationBuffer, "SubNegotitation - TN3270E FUNCTIONS REQUEST");

                response.append((uchar) IAC);
                response.append((uchar) SB);
                response.append((uchar) TELOPT_TN3270E);
                response.append((uchar) TN3270E_FUNCTIONS);
                response.append((uchar) TN3270E_IS);

                //TODO: Unsupported functions
                for(int i = 3; i < subNegotiationBuffer.size(); i++)
                {
//                    qDebug() << tn3270e_functions_strings[(int) subNeg.at(i)];
                    response.append(subNegotiationBuffer.at(i));
                }

                response.append((uchar) IAC);
                response.append((uchar) SE);

                dump(response, "SubNegotitation - TN3270E FUNCTIONS IS Response");

                dataStream.writeRawData(response, response.size());
                break;
            }
            dump(subNegotiationBuffer, "SubNegotitation - Unknown TN3270E request");
            break;
        default:
            dump(subNegotiationBuffer, "SubNegotitation - Unknown request");
            break;
    }
    telnetState = TELNET_STATE_DATA;

    // Finished processing this buffer, so clear it.
    subNegotiationBuffer.clear();
}


/**
 * @brief SocketConnection::dump
 *        Utility method to hexdump a buffer, formatted at 32 bytes, with ASCII character representation.
 * @param a - QByteArray to be dumped
 * @param title - A title to distinguish this from other hexdumps
 *
 */
void SocketConnection::dump(QByteArray &a, QString title)
{
    int w = 0;
    QString bytes;
    QString byteChars;

    qDebug() << "";
    qDebug().noquote() << title << " Start -------------------------------------------";

    for (int i = 0; i < a.size(); i++)
    {
        if (w > 31)
        {
            qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 |").arg(i - 31, 4, 16).arg(bytes.toUpper().leftJustified(96)).arg(byteChars.leftJustified(32));

            byteChars = "";
            bytes = "";
            w = 0;
        }

        bytes.append(QString("%1 ").arg((uchar)a.at(i), 2, 16, QLatin1Char('0')));

        if (isalnum(a.at(i)))
            byteChars.append(a.at(i));
        else
            byteChars.append(".");

        w++;

    }

    if (w != 0)
    {
        qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 |").arg(a.size() > 31 ? a.size() - 31 : 0, 4, 16).arg(bytes.toUpper().leftJustified(96)).arg(byteChars.leftJustified(32));
    }

    qDebug().noquote() << title << " End   -------------------------------------------";
    qDebug() << "";

}
