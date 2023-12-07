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

#ifndef CELL_H
#define CELL_H

#include <QGraphicsSimpleTextItem>
#include <QObject>
#include <QGraphicsScene>

#include "ColourTheme.h"
#include "CodePage.h"

class Cell : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    Cell(qreal x_pos, qreal y_pos, qreal x, qreal y, CodePage &cp, ColourTheme::Colours &palette, QGraphicsItem *parent, QGraphicsScene *scene);

    QRectF boundingRect() const;

    // Attributes that may have character-specific attributes
    enum CharAttr
    {
        EXTENDED,
        COLOUR,
        CHARSET,
        TRANSPARENCY
    };

    int type() const;

    void setChar(uchar ebcdic);
    void setCharFromKB(uchar ascii);

    // Getters, inline for speed
    inline uchar getEBCDIC()                        { return ebcdic; };
    inline QChar getChar()                          { return glyph.text().at(0); }

    inline ColourTheme::Colour getColour()          { return colNum; };

    inline bool isFieldStart()                      { return fieldStart; };
    inline bool isAutoSkip()                        { return prot & num; };
    inline bool isNumeric()                         { return num; };
    inline bool isGraphic()                         { return graphic; };
    inline bool isMdtOn()                           { return mdt; };
    inline bool isProtected()                       { return prot; };
    inline bool isDisplay()                         { return display; };
    inline bool isPenSelect()                       { return pen; };
    inline bool isIntensify()                       { return intensify; };
    inline bool isExtended()                        { return extended; };
    inline bool isUScore()                          { return uscore; };
    inline bool isReverse()                         { return reverse; };
    inline bool isBlink()                           { return blink; };

    bool hasCharAttrs(Cell::CharAttr ca);

    // Setters
    void setColour(ColourTheme::Colour c);
    void setFieldStart(bool fs);
    void setNumeric(bool n);
    void setGraphic(bool ge);
    void setMDT(bool m);
    void setProtected(bool p);
    void setDisplay(bool d);
    void setPenSelect(bool p);
    void setIntensify(bool i);
    void setExtended(bool e);
    void setUScore(bool u);
    void setReverse(bool r);
    void setBlink(bool b);

    void setFont(QFont f);

    void setCharAttrs(Cell::CharAttr ca, bool c);
    void resetCharAttrs();

    void copy(Cell &c);

    void setAttrs(bool prot, bool mdt, bool num, bool pensel, bool blink, bool disp, bool under, bool rev, ColourTheme::Colour col);

    void refreshCodePage();

    void blinkChar(bool blink);

    void updateCell();

private:

    QGraphicsLineItem underscore;
    QGraphicsSimpleTextItem glyph;

    qreal xsize;
    qreal ysize;

    // Size
    qreal xscale;
    qreal yscale;

    ColourTheme::Colours &palette;

    // Codepage
    CodePage &cp;

    // EBCDIC code for this character
    uchar ebcdic;

    // Is this a GE character?
    bool graphic;

    // Is this a field start?
    bool fieldStart;

    // Field attributes
    bool num;
    bool mdt;
    bool prot;
    bool display;
    bool pen;
    bool intensify;

    /* Extended Attributes */
    bool extended;
    bool uscore;
    bool reverse;
    bool blink;

    /* Character Attributes in effect */
    bool charAttrExtended;
    bool charAttrColour;
    bool charAttrCharSet;
    bool charAttrTransparency;

    // Colour of glyph
    ColourTheme::Colour colNum;

    // Cell changed with this data transmission
    bool changed;
};

#endif // CELL_H
