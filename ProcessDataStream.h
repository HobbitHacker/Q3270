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

#ifndef PROCESSDATASTREAM_H
#define PROCESSDATASTREAM_H

#include <qglobal.h>
#include <QGuiApplication>
#include <QScreen>

#include <stdlib.h>
#include <QObject>
#include <QDebug>

#include <arpa/telnet.h>
#include <map>

#include "DisplayScreen.h"

class Terminal;

class ProcessDataStream : public QObject
{
	Q_OBJECT
	
	public:

        QString EBCDICtoASCII();

        bool processing;

        ProcessDataStream(Terminal *t);

        void showFields();
        void resetMDTs();

    public slots:

        void processStream(QByteArray &b, bool tn3270e);

    signals:

        void bufferReady(QByteArray &b);
        void processingComplete();
        void unlockKeyboard();
        void blink();
        void disconnected();

    private:

        DisplayScreen *screen;

        Terminal *terminal;

        QByteArray::Iterator buffer;

        /* Which screen size we're currently using */
        bool alternate_size;

        /* Screen position according to incoming 3270 data stream */
        int primary_pos;

        /* Dimensions of currently active screen */
        int screen_x;
        int screen_y;
        int screenSize;

        bool resetMDT;
        bool restoreKeyboard;
        bool alarm;
        int lastAID;    // Last AID encountered

        bool wsfProcessing;
        int wsfLen;

        // True if the previous byte/byte sequence was a command; used for PT processing
        bool lastWasCmd;

        // True if the last Program Tab operation hit the end of the display
        bool lastPTwrapped;

        void setScreen(bool alternate = false);

        void placeChar();
        void placeChar(uchar c);

        void processWCC();
        void processOrders();

        /* 3270 Command Codes */
        /* TODO: 3270 Command codes RMA and EAU */
        void processW();
        void processEW(bool alternate);      // Incorporates EWA
        void processRB();
        void processWSF();
        void processRM();

        /* 3270 Orders */
        void processSF();
        void processSFE();
        void processSBA();
        void processSA();
        void processMF();
        void processIC();
        void processPT();
        void processRA();
        void processEUA();
        void processGE();

        int extractBufferAddress();
        void incPos();

        void WSFreset();
        void WSFreadPartition();
        void WSFoutbound3270DS();

        void replySummary(QByteArray &buffer);
        void addBytes(QByteArray &buffer, uchar *bytes, int len);
        void processAttributePairs(int mode);
};

#endif // PROCESSDATASTREAM_H
