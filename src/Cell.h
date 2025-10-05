/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef CELL_H
#define CELL_H

#include <QGraphicsSimpleTextItem>
#include <QObject>
#include <QGraphicsScene>
#include <QPointer>

#include "Q3270.h"
#include "Models/Colours.h"
#include "CodePage.h"


class Cell : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    Cell(int celladdress, qreal x_pos, qreal y_pos, qreal x, qreal y, CodePage &cp, const Colours *palette, QGraphicsItem *parent, QGraphicsScene *scene);

    //QRectF boundingRect() const;

    void setChar(const uchar ebcdic);
    void setCharFromKB(const uchar c);

    // Getters, inline for speed
    inline uchar getEBCDIC() const                  { return ebcdic; };
    inline QChar getChar() const                    { return glyph.text().at(0); }

    inline Q3270::Colour getColour() const          { return colNum; };

    inline bool isFieldStart() const                { return fieldStart; };
    inline bool isAutoSkip() const                  { return prot & num; };
    inline bool isNumeric() const                   { return num; };
    inline bool isGraphic() const                   { return graphic; };
    inline bool isMdtOn() const                     { return mdt; };
    bool isProtected() const;
    inline bool isDisplay() const                   { return display; };
    inline bool isPenSelect() const                 { return pen; };
    inline bool isIntensify() const                 { return intensify; };
    inline bool isExtended() const                  { return extended; };
    bool isUScore() const;
    inline bool isReverse() const                   { return reverse; };
    inline bool isBlink() const                     { return blink; };

    int getField() const;

    bool hasCharAttrs(const Q3270::CharAttr) const;

    // Setters
    void setField(Cell *c);

    void setColour(const Q3270::Colour col);
    void setColourFromField()                       { setColour(field->getColour()); };

    void setFieldStart(const bool fs);
    void setNumeric(const bool num);
    void setGraphic(const bool ge);
    void setMDT(const bool mdt);
    void setProtected(const bool prot);
    void setDisplay(const bool display);
    void setPenSelect(const bool pensel);
    void setIntensify(const bool intens);
    void setExtended(const bool extend);
    void setUnderscore(const bool uscore);
    void setReverse(const bool reverse);
    void setBlink(const bool blink);

    void setFont(QFont);

    void setCharAttrs(Q3270::CharAttr, bool);
    void resetCharAttrs();

    void copy(Cell &);

    void setAttrs(bool, bool, bool, Q3270::Colour);

    void refreshCodePage();

    void blinkChar(bool);
     bool updateCell();

private:

    QGraphicsLineItem underscore;
    QGraphicsSimpleTextItem glyph;

    // Screen position of this cell
    int address;

    qreal xsize;
    qreal ysize;

    // Size
    qreal xscale;
    qreal yscale;

    const Colours *palette;

    // Codepage
    const CodePage &cp;

    // EBCDIC code for this character
    uchar ebcdic;

    // Is this a GE character?
    bool graphic;

    // Is this a field start?
    bool fieldStart;

    // Field position
    Cell *field;

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
    Q3270::Colour colNum;

    // Cell changed with this data transmission
    bool changed;
    bool displayChanged;
    bool colourChanged;
    bool reverseChanged;
    bool uscoreChanged;
    bool glyphChanged;

    int updateCount;
};

#endif // CELL_H
