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

#include <text.h>

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

/* Special handling needed for these characters */
#define IBM3270_CHAR_NULL 0x00
/**
 * @todo write docs
 */
class DisplayDataStream : public QObject
{
	Q_OBJECT
	
	public:
	
        DisplayDataStream(QGraphicsScene *parent = nullptr);
        QString EBCDICtoASCII();
        void processStream();
        bool processing;
        void addByte(uchar b);

        void insertChar(QString keycode);
        void moveCursor(int x, int y, bool absolute = false);
        void eraseField();

        void showFields();
        int getCursorAddress();
        void findNextUnprotected();
        void findFirstUnprotected();

        typedef struct {
            bool askip;
            bool prot;
            bool mdt;
        } FieldFlags;


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

        uchar *buffer;
        uchar *bufferCurrentPos;

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

        Text *glyph[SCREENX * SCREENY];
        QGraphicsRectItem *cells[SCREENX * SCREENY];

        struct {
                bool mdt;
                bool prot;
                bool startField;
                bool askip;
        } attributes[SCREENX * SCREENY];

        std::map<int, FieldFlags> screenFields;

        QGraphicsRectItem *cursor;
        QLabel *cursorAddress;

        void placeChar(uchar *b);

        uchar *processEW(uchar *b);
        uchar *processSF(uchar *b);
        uchar *processSBA(uchar *b);
        uchar *processSFE(uchar *b);
        uchar *processRA(uchar *b);

        void processIC();

        int extractBufferAddress(uchar *b);

        void setAttributes(uchar *b);

        void removeCursor();
        void addCursor();

        std::map<int, FieldFlags>::iterator findField(int pos);
};

#endif // DISPLAYDATASTREAM_H
