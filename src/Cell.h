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

#include "Q3270.h"
#include "Models/Colours.h"

class Cell
{

public:
    Cell();

    void setChar(const uchar ebcdic);

    // Inline getters, for speed
    uchar getEBCDIC() const                  { return ebcdic; }
    bool isFieldStart() const                { return fieldStart; }
    bool isAutoSkip() const                  { return prot & num; }
    bool isNumeric() const                   { return num; }
    bool isGraphic() const                   { return graphic; }
    bool isMdtOn() const                     { return mdt; }
    bool isPenSelect() const                 { return pen; }
    bool isIntensify() const                 { return intensify; }
    bool isExtended() const                  { return extended; }

    bool isUScore()  const                   { return getHighlight() == Q3270::Underscore; }
    bool isReverse() const                   { return getHighlight() == Q3270::Reverse; }
    bool isBlink()   const                   { return getHighlight() == Q3270::Blink; }

    Q3270::Highlight getHighlight() const;;

    Q3270::Colour getColour() const;

    bool isProtected() const;
    bool isDisplay() const;

    Cell* getField();

    bool hasCharAttrs(const Q3270::CharAttr) const;

    // Setters
    void setField(Cell *c);
    void setColour(const Q3270::Colour col);
    void setFieldStart(const bool fs);
    void setNumeric(const bool num);
    void setGraphic(const bool ge);
    void setMDT(const bool mdt);
    void setProtected(const bool prot);
    void setDisplay(const bool display);
    void setPenSelect(const bool pensel);
    void setIntensify(const bool intens);
    void setExtended(const bool extend);
    void setHighlight(const Q3270::Highlight h);

    void setCharAttrs(Q3270::CharAttr, bool);
    void resetCharAttrs();

    void copy(const Cell &);

private:

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
    Q3270::Highlight highlight;

    /* Character Attributes in effect */
    bool charAttrExtended;
    bool charAttrColour;
    bool charAttrCharSet;
    bool charAttrTransparency;

    // Colour of glyph
    Q3270::Colour colNum;
};

#endif // CELL_H
