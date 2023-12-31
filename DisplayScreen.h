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

#ifndef DISPLAYDATA_H
#define DISPLAYDATA_H

#include <stdlib.h>
#include <string.h>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QDebug>
#include <QTimer>
#include <QObject>

#include "Cell.h"
#include "ColourTheme.h"
#include "CodePage.h"

class DisplayScreen : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

    public:

    DisplayScreen(int screen_x, int screen_y, CodePage &cp, ColourTheme::Colours &palette, QGraphicsScene *scene);
        ~DisplayScreen();

        void mousePressEvent(QGraphicsSceneMouseEvent *mEvent);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *mEvent);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *mEvent);

        int width();
        int height();
        qreal gridWidth();
        qreal gridHeight();

        void setChar(int pos, short unsigned int c, bool fromKB);
        void setCharAttr(unsigned char c, unsigned char d);

        void resetExtendedHilite(int pos);
        void resetExtended(int pos);
        void resetCharAttr();
        void resetColours();
        void resetMDTs();

        void setExtendedColour(int pos, bool foreground, unsigned char c);

        void setExtendedBlink(int pos);
        void setExtendedReverse(int pos);
        void setExtendedUscore(int pos);

        void setField(int pos, unsigned char c, bool sfe);
        void setGraphicEscape();

        void getModifiedFields(QByteArray &buffer);

        int findNextUnprotectedField(int pos);
        int findPrevUnprotectedField(int pos);

        bool insertChar(unsigned char c, bool insertMode);

        void eraseUnprotected(int start, int end);

        void setCursor(int x, int y);
        void setCursor(int pos);
        void showCursor();
        void cascadeAttrs(int startpos);

        unsigned char getChar(int pos);

        bool isAskip(int pos);
        bool isProtected(int pos);
        bool isFieldStart(int pos);

        void clear();
        void setFont(QFont font);
        void setFontScaling(bool fontScaling);

        void toggleRuler();
        void setRuler();
        void rulerMode(bool on);
        void setRulerStyle(int rulerStyle);

        void getScreen(QByteArray &buffer);
        void readBuffer();

        void addPosToBuffer(QByteArray &buffer, int pos);

        void refresh();

        void dumpFields();
        void dumpDisplay();
        void dumpInfo();

    signals:

        void bufferReady(QByteArray &buffer);

    public slots:

        void blink();
        void cursorBlink();
        void setStatusXSystem(QString text);
//        void showStatusCursorPosition(int x, int y);
        void setStatusInsert(bool ins);
        void setCursorColour(bool inherit);
        void setCodePage();
        void copyText();

        void moveCursor(int x, int y);
        void deleteChar();
        void newline();
        void tab(int offset);
        void backtab();
        void home();
        void backspace();
        void eraseEOF();
        void endline();

        void processAID(int aid, bool shortread);
        void interruptProcess();

    private:

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

        CodePage &cp;
        ColourTheme::Colours &palette;

        const char *colName[12] = { "black", "blue", "red", "magenta", "green", "cyan", "yellow", "neutral",
                                    "protected", "unprotected,intensfied", "unprotected", "protected, intensified"};

        int screen_x;                /* Max Columns */
        int screen_y;                /* Max Rows */
        int screenPos_max;           /* Max position on screen */

        int cursor_pos;              /* Cursor position */

//        QVector<Glyph *> glyph;               /* Character on screen */
        QVector<Cell *> cell;    /* Screen slot */
//        QVector<QGraphicsLineItem *> uscore;  /* Underscores */

        bool blinkShow;             /* Whether the character is shown/hidden for a given blink event */
        bool cursorShow;            /* Whether the cursor is shown/hidden for a given blink event */
        bool cursorColour;          // Whether cursor inherits the colour of the character underneath

        bool geActive;              // Next character is Graphic Escape

        bool rulerOn;               // Whether ruler is displayed
        int ruler;                  // Style of ruler

        bool unformatted;           // True if no fields are defined

        int lastAID;                // Last attention key identifier

        /* Character Attributes in effect */
        bool useCharAttr;

        struct {
                bool uscore;
                bool uscore_default;

                bool reverse;
                bool reverse_default;

                bool blink;
                bool blink_default;

                QBrush colour;
                ColourTheme::Colour colNum;
                bool colour_default;
        } charAttr;

        // Cursor
        QGraphicsRectItem cursor;
        QGraphicsLineItem crosshair_X;
        QGraphicsLineItem crosshair_Y;

        // Status bar
        QGraphicsLineItem statusBar;
        QGraphicsSimpleTextItem statusConnect;
        QGraphicsSimpleTextItem statusXSystem;
        QGraphicsSimpleTextItem statusCursor;
        QGraphicsSimpleTextItem statusInsert;

        qreal gridSize_X;
        qreal gridSize_Y;

        QGraphicsRectItem *myRb;
        QPointF mouseStart;

        int findField(int pos);
        int findNextField(int pos);

};

#endif // DISPLAYDATA_H
