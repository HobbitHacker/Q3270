#ifndef DISPLAYDATA_H
#define DISPLAYDATA_H

#include <stdlib.h>
#include <string.h>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <QDebug>
#include <QTimer>

#include "Glyph.h"
#include "ColourTheme.h"
#include "CodePage.h"

class DisplayScreen : public QGraphicsScene
{
    Q_OBJECT

    public:

        DisplayScreen(int screen_x, int screen_y, ColourTheme::Colours, CodePage *cp);
        ~DisplayScreen();

        int width();
        int height();
        qreal gridWidth();
        qreal gridHeight();

        void setChar(int pos, short unsigned int c, bool move, bool fromKB);
        void setCharAttr(unsigned char c, unsigned char d);

        void resetExtendedHilite(int pos);
        void resetExtended(int pos);
        void resetCharAttr();
        void resetColours();
        int resetFieldAttrs(int start);
        void resetMDTs();

        void setExtendedColour(int pos, bool foreground, unsigned char c);
        void setExtendedHilite(int pos);
        void setExtendedBlink(int pos);
        void setExtendedReverse(int pos);
        void setExtendedUscore(int pos);
        void setColour(int pos, bool foreground, unsigned char c);
        void setField(int pos, unsigned char c, bool sfe);
        void setGraphicEscape();

        void getModifiedFields(QByteArray &buffer);

        int findNextUnprotectedField(int pos);
        int findPrevUnprotectedField(int pos);

        bool insertChar(int pos, unsigned char c, bool insertMode);
        void deleteChar(int pos);
        void eraseEOF(int pos);

        void eraseUnprotected(int start, int end);

        void setCursor(int pos);
        void showCursor();
        void setFieldAttrs(int startPos);
        void cascadeAttrs(int startpos);

        unsigned char getChar(int pos);

        bool isAskip(int pos);
        bool isProtected(int pos);
        bool isFieldStart(int pos);

        void clear();
        void setFont(QFont font);
        void setColourPalette(ColourTheme::Colours c);
        void setFontScaling(bool fontScaling);

        void toggleRuler();
        void drawRuler(int x, int y);
        void setRuler();
        void rulerMode(bool on);
        void setRulerStyle(int rulerStyle);

        void getScreen(QByteArray &buffer);

        void addPosToBuffer(QByteArray &buffer, int pos);

        void dumpFields();
        void dumpDisplay();
        void dumpInfo(int pos);

    public slots:

        void blink();
        void cursorBlink();
        void setStatusXSystem(QString text);
        void showStatusCursorPosition(int x,int y);
        void setStatusInsert(bool ins);
        void setCursorColour(bool inherit);
        void setCodePage();

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

        ColourTheme::Colours palette;

        CodePage *cp;

        const char *colName[12] = { "black", "blue", "red", "magenta", "green", "cyan", "yellow", "neutral",
                                    "protected", "unprotected,intensfied", "unprotected", "protected, intensified"};

        int screen_x;                /* Max Columns */
        int screen_y;                /* Max Rows */
        int screenPos_max;           /* Max position on screen */

        QVector<Glyph *> glyph;               /* Character on screen */
        QVector<QGraphicsRectItem *> cell;    /* Screen slot */
        QVector<QGraphicsLineItem *> uscore;  /* Underscores */

        bool blinkShow;             /* Whether the character is shown/hidden for a given blink event */
        bool cursorShow;            /* Whether the cursor is shown/hidden for a given blink event */
        bool cursorColour;          // Whether cursor inherits the colour of the character underneath

        bool geActive;              // Next character is Graphic Escape

        bool rulerOn;               // Whether ruler is displayed
        int ruler;                  // Style of ruler


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

        QGraphicsRectItem cursor;

        QGraphicsLineItem crosshair_X;
        QGraphicsLineItem crosshair_Y;
        QGraphicsLineItem statusBar;

        QGraphicsSimpleTextItem statusConnect;
        QGraphicsSimpleTextItem statusXSystem;
        QGraphicsSimpleTextItem statusCursor;
        QGraphicsSimpleTextItem statusInsert;

        QFont termFont;
        bool fontScaling;            // Font scales with cell size

        qreal gridSize_X;
        qreal gridSize_Y;

        int findField(int pos);
        int findNextField(int pos);
        int findPrevField(int pos);
};

#endif // DISPLAYDATA_H
