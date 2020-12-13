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
#include <qobject.h>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QLabel>
#include <QHostInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

#include <stdlib.h>
#include <QObject>
#include <QDebug>
#include <chrono>
#include <thread>

#include "text.h"
#include "Buffer.h"
#include "DisplayScreen.h"
#include "3270.h"
#include "Terminal.h"
#include <arpa/telnet.h>

#include <map>

/**
 * TODO write docs
 */
class ProcessDataStream : public QObject
{
	Q_OBJECT
	
	public:
	
        ProcessDataStream(QGraphicsScene *parent, Terminal *t);
        QString EBCDICtoASCII();
        void processStream(Buffer *b);
        bool processing;

        void insertChar(unsigned char keycode, bool insMode);

        void moveCursor(int x, int y, bool absolute = false);

        void tab(int offset = 1);
        void backtab();
        void home();
        void eraseField();
        void deleteChar();
        void newline();
        void toggleRuler();
        void setFont(QFont font);

        void showFields();

        void resetMDTs();

        void processAID(int aid, bool shortRead);
        void interruptProcess();

        int getCursorAddress(int offset = 0);

        typedef struct {
            bool askip;
            bool prot;
            bool mdt;
        } FieldFlags;

        DisplayScreen *screen;

    signals:

        void bufferReady(Buffer *buffer);
        void keyboardUnlocked();
        void cursorMoved(int x, int y);
        void blink();
        void disconnected();

	private:

        QGraphicsScene *scene;
        QGraphicsView *terminal;

        Terminal *term;

        DisplayScreen *default_screen;
        DisplayScreen *alternate_screen;

        QTimer *blinker;

        uchar *buffer;
        uchar *bufferCurrentPos;

        typedef std::map<int, FieldFlags>::iterator FieldIterator;

        int bufferLength;

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

        QLabel *cursorAddress;

        void setScreen(bool alternate = false);

        void placeChar(Buffer *b);
        void placeChar(int c);

        void processWCC(Buffer *b);
        void processOrders(Buffer *b);

        /* 3270 Command Codes */
        /* TODO: 3270 Command codes RB, RMA and EAU */
        void processW(Buffer *b);
        void processEW(Buffer *b, bool alternate);      // Incorporates EWA
        void processWSF(Buffer *b);
        void processRM();


        /* 3270 Orders */
        /* TODO: 3270 Orders MF, PT and GE */
        void processSF(Buffer *b);
        void processSFE(Buffer *b);
        void processSBA(Buffer *b);
        void processSA(Buffer *b);
        void processIC();
        void processRA(Buffer *b);
        void processEUA(Buffer *b);


        int extractBufferAddress(Buffer *b);
        void incPos();

        void removeCursor();
        void addCursor();

        bool wsfProcessing;
        int wsfLen;

        void WSFreset(Buffer *b);
        void WSFreadPartition(Buffer *b);
        void WSFoutbound3270DS(Buffer *b);

        void replySummary(Buffer *buffer);
};

#endif // DISPLAYDATASTREAM_H
