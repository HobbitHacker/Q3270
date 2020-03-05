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

#include "text.h"
#include "buffer.h"
#include "DisplayData.h"

#include <map>

#define SCREENX 80
#define SCREENY 24
#define GRIDSIZE_X 9
#define GRIDSIZE_Y 16

/* 3270 Write Commands */
#define IBM3270_W    0xF1  /* Write */
#define IBM3270_EW   0xF5  /* Erase Write */
#define IBM3270_EWA  0x7E  /* Erase/Write Alternate */
#define IBM3270_RB   0xF2  /* Read Buffer */
#define IBM3270_RM   0xF6  /* Read Modified */
#define IBM3270_RMA  0x6E  /* Read Modified Alternate */
#define IBM3270_EAU  0x6F  /* Erase All Uprotected */
#define IBM3270_WSF  0xF3  /* Write Structured Field */

/* 3270 Orders */
#define IBM3270_SF   0x1D   /* Start Field */
#define IBM3270_SBA  0x11   /* Set Buffer Address */
#define IBM3270_SFE  0x29   /* Start Field Extended */
#define IBM3270_IC   0x13   /* Insert Cursor */
#define IBM3270_RA   0x3C   /* Repeat to Address */
#define IBM3270_SA   0x28   /* Set Attribute */

/* Constants for some EBCDIC chars */
#define IBM3270_CHAR_NULL  0x00
#define IBM3270_CHAR_SPACE 0x40

/* 3270 AIDs */
#define IBM3270_AID_ENTER  0x7D

/* Write Structured Field Commands */
#define IBM3270_WSF_RESET         0x00
#define IBM3270_WSF_READPARTITION 0x01

#define IBM3270_WSF_QUERYREPLY    0x88

#define IBM3270_EXT_DEFAULT       0x00
#define IBM3270_EXT_HILITE        0x41
#define IBM3270_EXT_FG_COLOUR     0x42
#define IBM3270_EXT_BG_COLOUR     0x45

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

        void insertChar(QString keycode, bool insMode);

        void moveCursor(int x, int y, bool absolute = false);

        void tab();
        void home();
        void eraseField();
        void deleteChar();

        void showFields();

        void resetMDTs();

        Buffer *processFields();

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

        const uchar EBCDICtoASCIImap[256] =
        {
            0x00, 0x01 ,0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F,  /* 00 - 07 */
            0x1A, 0x1A ,0x1A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 09 - 0F */
            0x10, 0x11 ,0x12, 0x13, 0x1A, 0x1A, 0x08, 0x1A,  /* 10 - 17 */
            0x18, 0x19 ,0x1A, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F,  /* 18 - 1F */
            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x0A, 0x17, 0x1B,  /* 20 - 27 */
            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x05, 0x06, 0x07,  /* 28 - 2F */
            0x1A, 0x1A ,0x16, 0x1A, 0x1A, 0x1A, 0x1A, 0x04,  /* 30 - 37 */
            0x1A, 0x1A ,0x1A, 0x1A, 0x14, 0x15, 0x1A, 0x1A,  /* 38 - 3F */
            0x20, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 40 - 47 */
            0x1A, 0x1A ,0x5B, 0x2E, 0x3C, 0x28, 0x2B, 0x21,  /* 48 - 4F */
            0x26, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 50 - 57 */
            0x1A, 0x1A ,0x5D, 0x24, 0x2A, 0x29, 0x3B, 0x5E,  /* 58 - 5F */
            0x2D, 0x2F ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 60 - 67 */
            0x1A, 0x1A ,0x7C, 0x2C, 0x25, 0x5F, 0x3E, 0x3F,  /* 68 - 6F */
            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 70 - 77 */
            0x1A, 0x60 ,0x3A, 0x23, 0x40, 0x27, 0x3D, 0x22,  /* 78 - 7F */
            0x1A, 0x61 ,0x62, 0x63, 0x64, 0x65, 0x66, 0x67,  /* 80 - 87 */
            0x68, 0x69 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 88 - 8F */
            0x1A, 0x6A ,0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,  /* 90 - 97 */
            0x71, 0x72 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* 98 - 9F */
            0x1A, 0x7E ,0x73, 0x74, 0x75, 0x76, 0x77, 0x78,  /* A0 - A7 */
            0x79, 0x7A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* A8 - AF */
            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* B0 - B7 */
            0x1A, 0x1A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* B8 - BF */
            0x7B, 0x41 ,0x42, 0x43, 0x44, 0x45, 0x46, 0x47,  /* C0 - C7 */
            0x48, 0x49 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* C8 - CF */
            0x7D, 0x4A ,0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,  /* D0 - D7 */
            0x51, 0x52 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* D8 - DF */
            0x5C, 0x1A ,0x53, 0x54, 0x55, 0x56, 0x57, 0x58,  /* E0 - E7 */
            0x59, 0x5A ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,  /* E8 - EF */
            0x30, 0x31 ,0x32, 0x33, 0x34, 0x35, 0x36, 0x37,  /* F0 - F7 */
            0x38, 0x39 ,0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A   /* F8 - FF */
        };

        const uchar ASCIItoEBCDICmap[256] =
        {
            0x00, 0x01, 0x02, 0x03, 0x1A, 0x09, 0x1A, 0x7F,  /* 00 - 07 */
            0x1A, 0x1A, 0x1A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,  /* 08 - 1F */
            0x10, 0x11, 0x12, 0x13, 0x3C, 0x3D, 0x32, 0x26,  /* 10 - 17 */
            0x18, 0x19, 0x3F, 0x27, 0x1C, 0x1D, 0x1E, 0x1F,  /* 18 - 1F */
            0x40, 0x4F, 0x7F, 0x7B, 0x5B, 0x6C, 0x50, 0x7D,  /* 20 - 27 */
            0x4D, 0x5D, 0x5C, 0x4E, 0x6B, 0x60, 0x4B, 0x61,  /* 28 - 2F */
            0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,  /* 30 - 37 */
            0xF8, 0xF9, 0x7A, 0x5E, 0x4C, 0x7E, 0x6E, 0x6F,  /* 38 - 3F */
            0x7C, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,  /* 40 - 47 */
            0xC8, 0xC9, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,  /* 48 - 4F */
            0xD7, 0xD8, 0xD9, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6,  /* 50 - 57 */
            0xE7, 0xE8, 0xE9, 0x4A, 0xE0, 0x5A, 0x5F, 0x6D,  /* 58 - 5F */
            0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,  /* 60 - 67 */
            0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,  /* 68 - 6F */
            0x97, 0x98, 0x99, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,  /* 70 - 77 */
            0xA7, 0xA8, 0xA9, 0xC0, 0x6A, 0xD0, 0xA1, 0x07,  /* 78 - 7F */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 80 - 87 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 80 - 8F */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 90 - 97 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* 98 - 9F */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* A0 - A7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* A8 - AF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* B0 - B7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* B8 - BF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* C0 - C7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* C8 - CF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* D0 - D7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* D8 - DF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* E0 - E7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* E8 - EF */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,  /* F0 - F7 */
            0x3F, 0x3F ,0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F   /* F8 - FF */
        };

        const unsigned char twelveBitBufferAddress[64] = {
            0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,  /* 00 0000 to 00 0011 */
            0xC8, 0xC9, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,  /* 00 0100 to 00 1111 */
            0x50, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,  /* 01 0000 to 01 0011 */
            0xD8, 0xD9, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,  /* 01 0100 to 01 1111 */
            0x60, 0x61, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,  /* 10 0000 to 10 0011 */
            0xE8, 0xE9, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,  /* 10 0100 to 10 1111 */
            0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,  /* 11 0000 to 11 0011 */
            0xF8, 0xF9, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,  /* 11 0100 to 11 1111 */
        };

        const QBrush palette[8] = {
            QColor(0,0,0),          /* Black */
            QColor(128,128,255),      /* Blue */
            QColor(255,0,0),        /* Red */
            QColor(255,0, 255),      /* Magenta */
            QColor(0,255,0),        /* Green */
            QColor(0,255,255),      /* Cyan */
            QColor(255,255,0),      /* Yellow */
            QColor(255,255,255)     /* White */
        };

        uchar *buffer;
        uchar *bufferCurrentPos;

        typedef std::map<int, FieldFlags>::iterator FieldIterator;

        int bufferLength;

        uchar *primaryDisplay;
        uchar *primaryDisplayPos;

        uchar *alternateDisplay;
        uchar *alternateDisplayPos;

        int primary_x, primary_y;
        int alternate_x, alternate_y;

        int cursor_x, cursor_y;

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

        Text *glyph[SCREENX * SCREENY];
        QGraphicsRectItem *cells[SCREENX * SCREENY];
        DisplayData *chars;

        struct {
                bool mdt;
                bool prot;
                bool startField;
                bool askip;
        } attributes[SCREENX * SCREENY];

        std::map<int, FieldFlags> screenFields;

        QGraphicsRectItem *cursor;
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

        void setAttributes(Buffer *b);

        void removeCursor();
        void addCursor();

        FieldIterator findNextUnprotected();
        FieldIterator findFirstUnprotected();


        std::map<int, FieldFlags>::iterator findField(int pos);

        void WSFreset(Buffer *b);
        void WSFreadPartition(Buffer *b);

        void replySummary(Buffer *buffer);
};

#endif // DISPLAYDATASTREAM_H