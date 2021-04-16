/*
 * Copyright 2020 Andy Styles <andy@styles.homeip.net>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DISPLAYDATASTREAM_H
#define DISPLAYDATASTREAM_H

#include <qglobal.h>
#include <QGuiApplication>
#include <QScreen>

#include <stdlib.h>
#include <QObject>
#include <QDebug>

#include "DisplayScreen.h"
#include "Q3270.h"
#include "TerminalView.h"
#include <arpa/telnet.h>

#include <map>

class TerminalTab;

/**
 * TODO write docs
 */
class ProcessDataStream : public QObject
{
	Q_OBJECT
	
	public:

        QString EBCDICtoASCII();

        bool processing;

        ProcessDataStream(TerminalView *t);

        void insertChar(unsigned char keycode, bool insMode);

        void moveCursor(int x, int y, bool absolute = false);

        void tab(int offset = 1);
        void backtab();
        void home();
        void eraseField();
        void deleteChar();
        void newline();
        void toggleRuler();
        void endline();

        void showFields();

        void resetMDTs();

        void processAID(int aid, bool shortRead);
        void interruptProcess();

    public slots:

        void processStream(QByteArray &b, bool tn3270e);

    signals:

        void bufferReady(QByteArray &b);
        void keyboardUnlocked();
        void cursorMoved(int x, int y);
        void blink();
        void disconnected();

    private:

        DisplayScreen *screen;

        TerminalView *terminal;

        QByteArray::Iterator buffer;

        /* Which screen size we're currently using */
        bool alternate_size;

        /* Screen position according to incoming 3270 data stream */
        int primary_x, primary_y;
        int primary_pos;

        /* Dimensions of currently active screen */
        int screen_x;
        int screen_y;
        int screenSize;

        /* Cursor position */
        int cursor_x, cursor_y;
        int cursor_pos;

        bool resetMDT;
        bool resetKB;
        bool alarm;
        int lastAID;    // Last AID encountered

        bool wsfProcessing;
        int wsfLen;

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
        /* TODO: 3270 Orders MF, PT */
        void processSF();
        void processSFE();
        void processSBA();
        void processSA();
        void processIC();
        void processRA();
        void processEUA();
        void processGE();


        void addCursorAddress(QByteArray &reply);
        int extractBufferAddress();
        void incPos();

        void removeCursor();
        void addCursor();

        void WSFreset();
        void WSFreadPartition();
        void WSFoutbound3270DS();

        void replySummary(QByteArray &buffer);
        void addBytes(QByteArray &buffer, uchar *bytes, int len);
};

#endif // DISPLAYDATASTREAM_H
