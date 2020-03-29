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

#include "text.h"
#include "buffer.h"
#include "DisplayData.h"
#include "3270.h"

#include <map>

#define SCREENX 80
#define SCREENY 24
#define GRIDSIZE_X 9
#define GRIDSIZE_Y 16

/**
 * @todo write docs
 */
class DisplayDataStream : public QObject
{
	Q_OBJECT
	
	public:
	
        DisplayDataStream(QGraphicsScene *parent = nullptr);
        QString EBCDICtoASCII();
        void processStream(Buffer *b);
        bool processing;

        void insertChar(unsigned char keycode, bool insMode);

        void moveCursor(int x, int y, bool absolute = false);

        void tab();
        void home();
        void eraseField();
        void deleteChar();
        void newline();

        void showFields();

        void resetMDTs();

        Buffer *processFields(int aid);

        int getCursorAddress(int offset = 0);

        typedef struct {
            bool askip;
            bool prot;
            bool mdt;
        } FieldFlags;

    signals:

        void bufferReady(Buffer *buffer);

	private:

        QGraphicsScene *display;
        DisplayData *screen;

        uchar *buffer;
        uchar *bufferCurrentPos;

        typedef std::map<int, FieldFlags>::iterator FieldIterator;

        int bufferLength;

        uchar *primaryDisplay;
        uchar *primaryDisplayPos;

        uchar *alternateDisplay;
        uchar *alternateDisplayPos;

        int primary_x, primary_y;
        int primary_pos;

        int alternate_x, alternate_y;

        int cursor_x, cursor_y;
        int cursor_pos;

        int count;

        int defaultScreenSize, alternateScreenSize;

        QBrush fieldAttr;
        bool mdt;
        bool prot;
        bool askip;

        bool resetKB;
        bool alarm;
        bool resetMDT;

        int wsfLen;

        struct {
            bool on;
            int foreground;
            int background;
            int highlight;
        } extended;

        DisplayData *chars;

        struct {
                bool mdt;
                bool prot;
                bool startField;
                bool askip;
        } attributes[SCREENX * SCREENY];

        QLabel *cursorAddress;

        void placeChar(Buffer *b);
        void placeChar(int c);

        void processWCC(Buffer *b);

        /* Write Commands */
        void processEW(Buffer *b);
        void processW(Buffer *b);
        void processSF(Buffer *b);
        void processSBA(Buffer *b);
        void processSFE(Buffer *b);
        void processRA(Buffer *b);
        void processSA(Buffer *b);
        void processEWA(Buffer *b);
        void processWSF(Buffer *b);

        void processIC();

        int extractBufferAddress(Buffer *b);

        void removeCursor();
        void addCursor();

        FieldIterator findNextUnprotected();
        FieldIterator findFirstUnprotected();

        void WSFreset(Buffer *b);
        void WSFreadPartition(Buffer *b);

        void replySummary(Buffer *buffer);
};

#endif // DISPLAYDATASTREAM_H
