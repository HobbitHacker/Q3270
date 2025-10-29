/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef PROCESSDATASTREAM_H
#define PROCESSDATASTREAM_H

#include <QGuiApplication>
#include <QScreen>

#include <QObject>
#include <QDebug>

#include <arpa/telnet.h>

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

        // Used to build replies to incoming commands (eg, RMx and inbound 3270 data streams)
        QByteArray reply;

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

        // True if the last command was a WRITE type command
        bool lastwasWrite;

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

        void replySummary();
        void addBytes(uchar *bytes, int len);
        void processAttributePairs(int mode);
};

#endif // PROCESSDATASTREAM_H
