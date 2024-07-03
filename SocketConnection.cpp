/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "SocketConnection.h"
#include "Q3270.h"

/**
 * @brief   SocketConnection::SocketConnection - handle incoming TCPIP data
 * @param   modelType - the terminal model type
 *
 * @details When a TCPIP connection is started, this routine handles the negotiation with the host. The
 *          terminal type is used when negotiating the capability of the host.
 *
 *          Signals are connected for when the socket is successfully opened, closed and when there is
 *          data to be read.
 */
SocketConnection::SocketConnection(int modelType)
{
    dataSocket = new QSslSocket(this);
//    dataSocket = new QTcpSocket(this);
    telnetState = Q3270::TELNET_STATE_DATA;

    this->termName = tn3270e_terminal_types[modelType];
	
    // Forward the connected and disconnected signals
    connect(dataSocket, &QSslSocket::connected, this, &SocketConnection::opened);
    connect(dataSocket, &QSslSocket::disconnected, this, &SocketConnection::closed);

    connect(dataSocket, &QSslSocket::readyRead, this, &SocketConnection::onReadyRead);
    // Forward the error signal, QOverload is necessary as error() is overloaded, see the Qt docs
    connect(dataSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred), this, &SocketConnection::error);

    tn3270e_Mode = false;
    secureMode = false;
    verifyCerts = false;
}

/**
 * @brief   SocketConnection::setSecure - indicate that this connection should be secured
 *
 * @details Signalled when the user selects the 'Secure Connection' option in the dialog
 */
void SocketConnection::setSecure(bool s)
{
    secureMode = s;
}

/**
 * @brief   SocketConnection::setVerify - indicate that the certificates should be verified for
 *          a secure connection.
 *
 * @details Signalled when the user selects the 'Verfiy Certficates' option in the dialog
 */
void SocketConnection::setVerify(bool v)
{
    verifyCerts = v;
}

/**
 *  @brief   SocketConnection:error - called when a socket error occurs
 *
 *  @details Signalled when a socket error happens, such as the remote host closing the
 *           connection.
 */
void SocketConnection::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Error:" << dataSocket->errorString();

    emit connectionEnded(dataSocket->errorString());
}

/**
 * @brief   SocketConnection::~SocketConnection - destructor
 *
 * @details Disconnect the signals that were established and delete the socket.
 */
SocketConnection::~SocketConnection()
{
    disconnect(dataSocket, &QSslSocket::readyRead, this, &SocketConnection::onReadyRead);
    disconnect(dataSocket, &QSslSocket::disconnected, this, &SocketConnection::closed);

    dataSocket->deleteLater();
}

/**
 * @brief   SocketConnection::opened - successful open of socket
 *
 * @details Slot called when the socket is successfully opened.
 */
void SocketConnection::opened()
{
    emit encryptedConnection(Q3270::Unencrypted);
    emit connectionStarted();

}

/**
 * @brief   SocketConnection::closed - socket has been closed
 *
 * @details Slot called when the socket has been closed.
 */
void SocketConnection::closed()
{
    emit connectionEnded();
    emit encryptedConnection(Q3270::Unencrypted);
}

/**
 * @brief   SocketConnection::disconnectMainframe - slot called when the connection is terminated
 *
 * @details Called when the user closes the connection.
 */
void SocketConnection::disconnectMainframe()
{
    disconnect(dataSocket, &QSslSocket::readyRead, this, &SocketConnection::onReadyRead);

    dataSocket->disconnectFromHost();

    dataSocket->close();
}

/**
 * @brief   SocketConnection::connectMainframe - called when the user connects to a host
 * @param   address - the target IP address
 * @param   port    - the target port
 * @param   luName  - the target LU name (may be empty)
 * @param   d       - the ProcessDataStream object
 *
 * @details Called when the user connects to a host. The address, port are used to establish the connection
 *          and the LU name is stored for later in the negotiation sequence. The ProcessDataStream object
 *          is passed so that the ProcessDataStream can pass data back to the host.
 */
void SocketConnection::connectMainframe(QString &address, quint16 port, QString luName, ProcessDataStream *d)
{
    certErrors = false;

    QList<QSslCipher> c = dataSocket->sslConfiguration().supportedCiphers();
    for(int i=0; i<c.size();i++)
    {
        qDebug() << c.at(i);
    }

    connect(dataSocket, &QSslSocket::stateChanged, this, &SocketConnection::socketStateChanged);
    connect(dataSocket, &QSslSocket::encrypted, this, &SocketConnection::socketEncrypted);
    connect(dataSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &SocketConnection::sslErrors);

    if (secureMode)
    {
        dataSocket->connectToHostEncrypted(address, port);
    }
    else
    {
        dataSocket->connectToHost(address, port);
    }

    qDebug() << "Encrypted:" << dataSocket->isEncrypted();
    qDebug() << "Certificate:" << dataSocket->peerCertificate();

//    disconnect(dataSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors), this, &SocketConnection::sslErrors);
//    dataSocket->connectToHost(address, port);
    displayDataStream = d;
    this->luName = luName;
}

/**
 * @brief   SocketConnection::sslErrors - handle SSL errors
 * @param   errors - the list of errors
 *
 * @details Called when SSL errors happen. If the user has chosen to not verify the certificates
 *          for an SSL connection, the connection is failed, otherwise, the SSL errors are
 *          enumerated and passed to Qt to be ignored, after which the connection can be
 *          considered 'semi secure'.
 */
void SocketConnection::sslErrors(const QList<QSslError> &errors)
{
    if (!verifyCerts)
    {
        dataSocket->ignoreSslErrors(errors);
        certErrors = true;
    }
    else
    {
        QString errs = "";
        for(int i = 0; i < errors.size(); i++)
        {
            errs.append(errors.at(i).errorString());
            qDebug() << "sslErrors:" << errors.at(i);
        }
        emit connectionEnded(errs);
    }
}

/**
 * @brief   SocketConnection::socketEncrypted
 * @param   errors - the list of errors
 *
 * @details Called when SSL errors happen, switches the padlock icon
 */
void SocketConnection::socketEncrypted()
{
    qDebug() << "Encypted!";
    if (certErrors)
    {
        emit encryptedConnection(Q3270::SemiEncrypted);
    }
    else
    {
        emit encryptedConnection(Q3270::Encrypted);
    }
}

QList<QSslCertificate> SocketConnection::getCertDetails()
{
    return dataSocket->peerCertificateChain();
}

/**
 * @brief   SocketConnection::socketStateChanged - display SSL errors
 * @param   state - socket state
 *
 * @details Display SSL errors. Called when a socket changes state.
 */
void SocketConnection::socketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "SocketStateChanged: " << state;

    QList<QSslError> e = dataSocket->sslHandshakeErrors();

    for(int i = 0; i < e.size(); i++)
    {
        qDebug() << "SocketStateChanged Handshake Errors:" << e.at(i);
    }
    switch(state)
    {
        case QAbstractSocket::UnconnectedState:
            emit encryptedConnection(Q3270::Unencrypted);
            break;
        case QAbstractSocket::ConnectedState:
            break;
        default:
            emit encryptedConnection(Q3270::Unencrypted);
    }
}

/**
 * @brief   SocketConnection::onReadyRead - process incoming TCPIP data
 *
 * @details This is the main driving routine for incoming TCPIP data. This routine handles all incoming
 *          traffic, and caters for the TN3270 negotiation.
 */
void SocketConnection::onReadyRead()
{

    qDebug() << "Encrypted: " << dataSocket->isEncrypted();

    // create a QDataStream operating on the socket
    QDataStream dataStream(dataSocket);

    // set the version so that programs compiled with different versions of Qt can agree on how to serialise
    dataStream.setVersion(QDataStream::Qt_5_12);

    // Incoming byte - unsigned
    uchar unsignedSocketByte;

    char socketByte;
    bool subNegotiationProcessing = false;

    QByteArray response;
    QString byteNotes;


    // start an infinite loop
    for (;;)
    {
        // Start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
        dataStream.startTransaction();

        response.clear();

        // Read incoming data, and convert to unsigned
        dataStream.readRawData(&socketByte, 1);
        unsignedSocketByte = (uchar) socketByte;

        if (dataStream.commitTransaction())
        {

            qDebug();

            switch (telnetState)
			{

                case Q3270::TELNET_STATE_DATA:
                    if (unsignedSocketByte == IAC)
					{
                        byteNotes.append("IAC ");
                        telnetState = Q3270::TELNET_STATE_IAC;
					} else 
					{
                        incomingData.append(unsignedSocketByte);
					}
					break;

                case Q3270::TELNET_STATE_IAC:
                    switch (unsignedSocketByte)
					{
                        case IAC: // double IAC (0xFF) means a single data byte 0xFF
                            if (subNegotiationProcessing)
                            {
                                // socketByte is 0xFF here, so we don't double up the incoming buffer
                                subNegotiationBuffer.append(unsignedSocketByte);
                                telnetState = Q3270::TELNET_STATE_SB;
                            }
                            else
                            {
                                // socketByte is 0xFF here, so we don't double up the incoming buffer
                                incomingData.append(unsignedSocketByte);
                                telnetState = Q3270::TELNET_STATE_DATA;
                            }
                            byteNotes.append("Double 0xFF ");;
							break;
						case DO:		// Request something, or confirm WILL request
                            byteNotes.append("DO ");
                            telnetState = Q3270::TELNET_STATE_IAC_DO;
							break;
						case DONT: 		// Request to not do something, or reject WILL request
                            byteNotes.append("DONT ");
                            telnetState = Q3270::TELNET_STATE_IAC_DONT;
							break;
						case WILL:  	// Offer to do something, or confirm DO request
                            byteNotes.append("WILL ");
                            telnetState = Q3270::TELNET_STATE_IAC_WILL;
							break;
						case WONT: 		// Reject DO request
                            byteNotes.append("WONT ");
                            telnetState = Q3270::TELNET_STATE_IAC_WONT;
							break;
						case SB:
                            byteNotes.append("SB ");
                            telnetState = Q3270::TELNET_STATE_SB;
                            subNegotiationProcessing = true;
							break;
                        case SE:
                            byteNotes.append("SE ");
                            if (subNegotiationProcessing)
                            {
                                qDebug() << byteNotes;
                                byteNotes = "";
                                processSubNegotiation();
                            }
                            else
                            {
                                byteNotes.append("- IAC SE, no SB? ");
                            }
                            subNegotiationProcessing = false;
                            telnetState = Q3270::TELNET_STATE_DATA;
                            break;
                        case EOR:
                            byteNotes.append("EOR ");
                            telnetState = Q3270::TELNET_STATE_DATA;
                            qDebug() << byteNotes;
                            byteNotes = "";
                            dump(incomingData,"Incoming Data");
                            emit dataStreamComplete(incomingData, tn3270e_Mode);
                            incomingData.clear();
							break;
						default:
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
							break;
					}
                    break;

                case Q3270::TELNET_STATE_IAC_DO:
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
                            telnetState = Q3270::TELNET_STATE_DATA;
							break;
						default:
                            response.append((uchar) IAC);
                            response.append((uchar) WONT);
                            response.append(unsignedSocketByte);
                            dataStream.writeRawData(response, 3);
                            telnetState = Q3270::TELNET_STATE_DATA;
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
                            break;
					}
					break;		

                case Q3270::TELNET_STATE_IAC_DONT:
                    switch(unsignedSocketByte)
                    {
                        case TELOPT_TN3270E:
                            tn3270e_Mode = false;
                            byteNotes.append("TN3270E off ");
                            break;
                        default:
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
                            telnetState = Q3270::TELNET_STATE_DATA;
                            break;
                    }
                    break;
                case Q3270::TELNET_STATE_IAC_WILL:
                    switch (unsignedSocketByte)
					{
						case TELOPT_BINARY:
						case TELOPT_EOR:
                            byteNotes.append("BINARY/EOR ");
                            response.append((uchar) IAC);
                            response.append((uchar) DO);
                            response.append(unsignedSocketByte);
                            dataStream.writeRawData(response, 3);
                            telnetState = Q3270::TELNET_STATE_DATA;
							break;
						default:
                            response.append((uchar) IAC);
                            response.append((uchar) DONT);
                            response.append(unsignedSocketByte);
                            dataStream.writeRawData(response, 3);
                            telnetState = Q3270::TELNET_STATE_DATA;
                            byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
							break;
					}
					break;

                case Q3270::TELNET_STATE_IAC_WONT:
                    byteNotes.append(QString("Not sure: %1 ").arg(unsignedSocketByte, 2, 16));
                    telnetState = Q3270::TELNET_STATE_DATA;
					break;

                case Q3270::TELNET_STATE_SB:
                    switch(unsignedSocketByte)
                    {
                        case IAC:
                            telnetState = Q3270::TELNET_STATE_IAC;
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

/**
 * @brief   SocketConnection::sendResponse - send data back to the host
 * @param   b - the byte array containing the data
 *
 * @details Send data back to the host. This may be prefixed with a TN3270E header if TN3270E negotiation
 *          happened earlier.
 */
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

/**
 * @brief   SocketConnection::processSubNegotiation - process a TN3270E sub-negotiation packet
 *
 * @details If the incoming packet was identified as a TN3270E sub-negotiation packet, this routine
 *          gets control and processes that packet, returning data to the host as needed.
 */
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
                break;
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
    telnetState = Q3270::TELNET_STATE_DATA;

    // Finished processing this buffer, so clear it.
    subNegotiationBuffer.clear();
}


/**
 * @brief   SocketConnection::dump - print out a buffer
 * @param   a     - QByteArray to be dumped
 * @param   title - A title to distinguish this from other hexdumps
 *
 * @details Debugging utility method to hexdump a buffer, formatted at 32 bytes, with EBCDIC/ASCII character
 *          representation.
 */
void SocketConnection::dump(QByteArray &a, QString title)
{
    
    CodePage ibm037 = CodePage();
    
    int w = 0;
    QString bytes;
    QString bytesASCII;
    QString bytesEBCDIC;

    qDebug() << "";
    qDebug().noquote() << "                         |---------------------------------------------------------------------------------------------|";
    qDebug().noquote() << QString(QString("                         | %1 Start ( %2 bytes)").arg(title).arg(a.length())).left(96);
    qDebug().noquote() << "                         |------------------------------------------------- Hex ---------------------------------------|  | ASCII                            | EBCDIC                           |";

    for (int i = 0; i < a.size(); i++)
    {
        if (w > 31)
        {
            qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4 |").arg(i - 31, 4, 16).arg(bytes.toUpper().leftJustified(96)).arg(bytesASCII.leftJustified(32)).arg(bytesEBCDIC.leftJustified(32));

            bytesASCII = "";
            bytesEBCDIC = "";
            bytes = "";
            w = 0;
        }
        
        bytes.append(QString("%1 ").arg((uchar)a.at(i), 2, 16, QLatin1Char('0')));
        
        uchar ebcdicchar = a.at(i);

        QString unicode = ibm037.getUnicodeChar(ebcdicchar);

        uchar latin1 = unicode.toLatin1().data()[0];

        if (isalnum(latin1))
            bytesEBCDIC.append(latin1);
        else
            bytesEBCDIC.append(".");

        if (isalnum(ebcdicchar))
            bytesASCII.append(ebcdicchar);
        else
            bytesASCII.append(".");

        w++;

    }

    if (w != 0)
    {
        qDebug().noquote() << QString("SocketConnection: %1 - %2 | %3 | %4 |").arg(a.size() > 31 ? a.size() - 31 : 0, 4, 16).arg(bytes.toUpper().leftJustified(96)).arg(bytesASCII.leftJustified(32)).arg(bytesEBCDIC.leftJustified(32));
    }

    qDebug().noquote() << "                         |---------------------------------------------------------------------------------------------|";
    qDebug().noquote() << QString(QString("                         | %1 End ( %2 bytes)").arg(title).arg(a.length())).left(98);
    qDebug().noquote() << "                         |------------------------------------------------- Hex ---------------------------------------|  | ASCII                            | EBCDIC                           |";
    qDebug() << "";

}
