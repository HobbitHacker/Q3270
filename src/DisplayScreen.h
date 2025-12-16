/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef DISPLAYSCREEN_H
#define DISPLAYSCREEN_H

#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QDebug>
#include <QtSvg>
#include <QTimer>
#include <QObject>

#include "Cell.h"
#include "CodePage.h"
#include "Q3270.h"
#include "Models/Colours.h"
#include "Display/ClickableSvgItem.h"
#include "Display/LockIndicator.h"

class DisplayScreen : public QGraphicsObject
{
    Q_OBJECT

    public:

    explicit DisplayScreen(int screen_x, int screen_y, CodePage &cp, const Colours *palette);
        ~DisplayScreen();

        void mousePressEvent(QGraphicsSceneMouseEvent *mEvent) override;
        void mouseMoveEvent(QGraphicsSceneMouseEvent *mEvent) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *mEvent) override;

        QRectF boundingRect() const override;

        void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override;


        int width() const;
        int height() const;
        qreal gridWidth() const;
        qreal gridHeight() const;

        void setSize(const int x, const int y);

        void setChar(int pos, uchar c, bool fromKB);
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

        void eraseUnprotected(int start, int end, Q3270::EraseResetMDT resetMDT);

        void setCursor(const int x, const int y);
        void setCursor(int pos);
        void showCursor();
        void cascadeAttrs(int startpos);

        bool isAskip(int pos) const;
        bool isProtected(int pos) const;
        bool isFieldStart(int pos) const;

        void clear();
        void setFont(const QFont &font);
        void setFontTweak(const Q3270::FontTweak f);

        void toggleRuler();
        void setRuler();
        void rulerMode(bool on);

        void setRulerStyle(Q3270::RulerStyle rulerStyle);

        void getScreen(QByteArray &buffer);
        void readBuffer();

        void addPosToBuffer(QByteArray &buffer, int pos);

        void dumpFields();
        void dumpDisplay();
        void dumpInfo();

    signals:

        void bufferReady(QByteArray &buffer);
        void cursorMoved(int x, int y);

    public slots:

        void blink();
        void cursorBlink();

        void setCursorColour(bool inherit);
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

        const CodePage &cp;
        const Colours *palette;

        int screen_x;                /* Max Columns */
        int screen_y;                /* Max Rows */
        int screenPos_max;           /* Max position on screen */

        int cursor_pos;              /* Cursor position */

        QVector<Cell> cells;    /* Screen slot */

        bool blinkShow;             /* Whether the character is shown/hidden for a given blink event */
        bool cursorShow;            /* Whether the cursor is shown/hidden for a given blink event */
        bool cursorColour;          // Whether cursor inherits the colour of the character underneath

        bool geActive;              // Next character is Graphic Escape

        bool rulerOn;               // Whether ruler is displayed
        Q3270::RulerStyle ruler;    // Style of ruler

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
                Q3270::Colour colNum;
                bool colour_default;
        } charAttr;

        // Cursor
        QGraphicsRectItem cursor;
        QGraphicsLineItem crosshair_X;
        QGraphicsLineItem crosshair_Y;

        qreal gridSize_X;
        qreal gridSize_Y;

        QRegion blinkCells;
        QGraphicsRectItem *myRb;
        QPointF mouseStart;

        QFont font;
        Q3270::FontTweak fontTweak;

        // Font tweak settings
        QPoint dotOffset;
        QPoint slashStart;
        QPoint slashEnd;
        int dotRadius;

        int findField(int pos);
        int findNextField(int pos);
        void applyCharAttributes(int pos, Cell *field);
        void updateFontMetrics();
};

#endif // DISPLAYSCREEN_H
