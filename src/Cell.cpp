/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "Cell.h"
#include <QDebug>
#include <QPen>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsItem>
#include <QGraphicsScene>

/*!
 * @brief
 * Cell represents a single character cell on the display matrix and all the attributes that
 * go with that (underscore, blink and so on).
 *
 * @details
 *
 * Each Cell contains attributes that are only relevant in particular situations - protection, display, numeric and
 * others can only be set by a Field Start, but given that any cell on the screen can be a field, all cells need to
 * have that potential setting.
 *
 * Each cell also contains a pointer to its Field Start cell (if one exists on the display), and field attributes
 * colours, etc are taken from that cell (if appropriate).
 */

Cell::Cell()
{

    field = nullptr;

    highlight = Q3270::NoHighlight;

    display = true;
    num = false;
    mdt = false;

    charAttrExtended = false;
    charAttrColour = false;
    charAttrCharSet = false;
    charAttrTransparency = false;
}

/**
 * @brief   Cell::isProtected - is this cell protected?
 * @return  true if the cell is protected, false if unprotected
 *
 * @details If this cell is not a field start, the value is taken from the field cell instead.
 */
bool Cell::isDisplay() const
{
    if (field && !fieldStart)
        return field->isDisplay();
    else
        return display;
}

/**
 * @brief Cell::setHighlight - set the highlight of the cell to the specified value
 * @param h - the new highlight value
 *
 * @details setHighlight is used to set the highlight of the cell to the specified value.
 *          Highlight is mutually exclusive, so setting one highlight will switch off the others.
 */
void Cell::setHighlight(Q3270::Highlight h)
{
    highlight = h;
}

/**
 * @brief   Cell::setChar - set the cell to the character specified
 * @param   ebcdic - the EBCDIC character code to be set
 */
void Cell::setChar(const uchar ebcdic)
{
    this->ebcdic = ebcdic;
}

/**
 * @brief   Cell::setColour - set the cell to colour specified
 * @param   c - the new colour
 *
 * @details This function sets the colour of the cell to the specified value
 */
void Cell::setColour(const Q3270::Colour c)
{
    colNum = c;
}

Q3270::Colour Cell::getColour() const
{
    return colNum;
};

/**
 * @brief   Cell::setFieldStart - set the 'Field' flag to show whether this cell is the start of a field
 * @param   fs - true for a field start, false for a normal character.
 *
 * @details setFieldStart() is called when the incoming data stream contains a SF or SFE order; it is also
 *          called when the cell used to be a field start, but that has now been overwritten.
 *
 *          Setting the cell to a Field Start causes underscore, reverse and blinking to be switched off.
 *          If the FieldStart is set, this cell's field pointer is set to null (otherwise the cell would
 *          point to itself).
 */
void Cell::setFieldStart(const bool fs)
{
    fieldStart = fs;

    if (fieldStart)
    {
        field = nullptr;
        setHighlight(Q3270::NoHighlight);

        if (!extended)
        {
            if (prot & !intensify)
            {
                colNum = Q3270::ProtectedNormal;
            }
            else if (prot & intensify)
            {
                colNum = Q3270::ProtectedIntensified;
            }
            else if (!prot & !intensify)
            {
                colNum = Q3270::UnprotectedNormal;
            }
            else
            {
                colNum = Q3270::UnprotectedIntensified;
            }
        }
    }
}

/**
 * @brief   Cell::setNumeric - set the cell to be numeric input only.
 * @param   n - true for numeric input only, false for normal.
 *
 * @details Setting a field to numeric input only should prevent any other characters from being entered.
 *
 * @warning Q3270 doesn't support numeric fields currently (ie, you can enter anything into a numeric field).
 */
void Cell::setNumeric(const bool n)
{
    num = n;
}

/**
 * @brief   Cell::setGraphic  - set the 'graphic escape' flag.
 * @param   ge - true for a graphic character, false for normal.
 *
 * @details setGraphic() is used to switch on (or off) the selection of the character from the internal 0310
 *          codepage, used for things like ISPF dialog box borders.
 */
void Cell::setGraphic(const bool ge)
{
    graphic = ge;
}

/**
 * @brief   Cell::setMDT - switch the MDT flag on or off
 * @param   m - true to turn it on, false to turn it off
 *
 * @details The MDT flag is used to indicate whether the user has modified a particular field. If this
 *          cell is not a field start, the value is taken from the field cell instead.
 */
void Cell::setMDT(const bool m)
{
    if (field)
    {
        field->mdt = m;
    }

    if (fieldStart)
    {
        mdt = m;
    }
}

/**
 * @brief   Cell::setProtected - switch protection on or off for this cell
 * @param   p - true for protected, false for unprotected
 *
 * @details Protected cells cannot be modified by the user. They are intended for text on the screen that
 *          is meant to guide the user (field prompts, menu items and so on).
 */
void Cell::setProtected(const bool p)
{
    prot = p;
}

/**
 * @brief   Cell::setDisplay - switch display on or offf for this cell
 * @param   d - true for display, false for non-display
 *
 * @details Non-display fields are used for things like passwords; although the user can enter data into
 *          the cell, it is not shown on screen.
 */
void Cell::setDisplay(const bool d)
{
    display = d;
}

/**
 * @brief   Cell::setPenSelect - set the lightpen-selectable on or off for the cell
 * @param   p - true for enabled, false for disabled
 *
 * @details Lightpen selectable fields used to be able to be triggered by lightpen that was pointed at
 *          the screen (I've never seen one) as a slightly more interactive way of dealing with a 3270
 *          display.
 *
 * @warning Q3270 doesn't support light pen selectable fields currently.
 */
void Cell::setPenSelect(const bool p)
{
    if (fieldStart)
    {
        pen = p;
    }
}

/**
 * @brief   Cell::setIntensify - set the intensity of the cell
 * @param   i - true for high-intensity, false for low-intensity
 *
 * @details In basic four colour mode, high-intensity fields are displayed in red (unprotected) or white (protected);
 *          low-intensity fields are displayed in blue (protected) or green (unprotected).
 */
void Cell::setIntensify(const bool i)
{
    intensify = i;
}

/**
 * @brief   Cell::setExtended - set the extended field status
 * @param   e - true for extended field, false otherwise.
 *
 * @details Extended fields can contain more colours than basic fields, and can contain additional
 *          hlighlighting like underscore, reverse or blinking.
 */
void Cell::setExtended(const bool e)
{
    extended = e;
}

/**
 *  @brief   Cell::setField - set this cell's field pointer
 *  @param   field - the address of the field in memory, or null
 *
 *  @details This routine is called when this cell's field position is changed. This occurs when the incoming
 *           3270 data stream defines a new field, or when an existing field is overwritten by the incoming
 *           data.
 *
 *           When the field is set (rather than being set to null), the cell colour is taken from the field,
 *           unless character attributes are in effect.
 */
void Cell::setField(Cell *field)
{
    this->field = field;

    if (field)
    {
        if (!charAttrColour)
        {
            colNum = field->colNum;
        }
    }
};

/**
 * @brief   Cell::getField - get the address of the field cell that owns this one
 * @return  The address of this cell's field within the 3270 matrix.
 *
 * @details When characters are entered into the display, either from the 3270 stream or from the keyboard,
 *          the field address is required to pick up the field attributes - if this field is a field start, then return
 *          this address, otherwise return the address of the field that owns this cell if there is one.
 */
Cell* Cell::getField()
{
    if (fieldStart)
    {
        return this;
    }
    if (field)
    {
        return field;
    }

    return nullptr;
};

/**
 * @brief   Cell::isProtected - return whether this field is protected or not
 * @return  True for a protected field, false otherwise
 *
 * @details Return the protected status of this cell. If this is a field start, then it's always
 *          protected.
 */
bool Cell::isProtected() const
{
    if (fieldStart)
    {
        return prot;
    }

    if (field)
    {
        return field->prot;
    }

    return false;
};


/**
 * @brief   Cell::getHighlight - return the highlight status of this cell
 *
 * @details Return the highlight status of this cell. If this is a field start,
 *          then it's always the field's highlight status.
 *
 * @return  the highlight status of this cell
 */
Q3270::Highlight Cell::getHighlight() const
{
    if (fieldStart || hasCharAttrs(Q3270::ExtendedAttr))
        return highlight;

    if (field)
        return field->highlight;

    return Q3270::NoHighlight;;
}

/**
 * @brief   Cell::setCharAttrs - set the characater attribute
 * @param   ca - the CharAttr to be set
 * @param   c - true to enable, false to disable
 *
 * @details Character attributes allow the display to have adjacent characters of different colours.
 *
 * @warning Q3270 does not handle any other character attribute than extended.
 */
void Cell::setCharAttrs(const Q3270::CharAttr ca, const bool c)
{
    switch(ca)
    {
        case Q3270::ExtendedAttr:
            charAttrExtended = c;
            break;
        case Q3270::ColourAttr:
            charAttrColour = c;
            break;
        case Q3270::CharsetAttr:
            charAttrCharSet = c;
            break;
        case Q3270::TransparencyAttr:
            charAttrTransparency = c;
    }
}

/**
 * @brief   Cell::hasCharAttrs - return the status of the specified character attribute
 * @param   ca - the CharAttr to be returne
 * @return  the state of the specified CharAttr
 *
 * @details This routine returns the state of the specified character attrbute for this Cell.
 */
bool Cell::hasCharAttrs(const Q3270::CharAttr ca) const
{
    switch(ca)
    {
        case Q3270::ExtendedAttr:
            return charAttrExtended;
        case Q3270::ColourAttr:
            return charAttrColour;
        case Q3270::CharsetAttr:
            return charAttrCharSet;
        case Q3270::TransparencyAttr:
            return charAttrTransparency;
    }

    //TODO: Should never happen, so throw an exception
    return false;
}

/**
 * @brief   Cell::resetCharAttrs - switch off all character attributes
 *
 * @details This function clears all character attributes. This is called when placing a character
 *          in a cell (the attributes may be added subsequently).
 */
void Cell::resetCharAttrs()
{
    charAttrExtended     = false;
    charAttrColour       = false;
    charAttrCharSet      = false;
    charAttrTransparency = false;
}

/**
 * @brief   Cell::copy - copy pertinent parts from another Cell
 * @param   fromCell - the source Cell
 *
 * @details When moving characters around on the screen through the keyboard (ie, through
 *          insert or delete actions), character attributes move with the character.
 */
void Cell::copy(const Cell &fromCell)
{
//    these should come from the Field Attribute
//    prot = fromCell.isProtected();
//    mdt = fromCell.isMdtOn();
//    num = fromCell.isNumeric();
//    pen = fromCell.isPenSelect();
//    setDisplay(fromCell.isDisplay());

    setColour(fromCell.getColour());
    setHighlight(fromCell.getHighlight());

    graphic = fromCell.isGraphic();

    setCharAttrs(Q3270::ExtendedAttr, fromCell.hasCharAttrs(Q3270::ExtendedAttr));
    setCharAttrs(Q3270::ColourAttr, fromCell.hasCharAttrs(Q3270::ColourAttr));
    setCharAttrs(Q3270::TransparencyAttr, fromCell.hasCharAttrs(Q3270::TransparencyAttr));
    setCharAttrs(Q3270::CharsetAttr, fromCell.hasCharAttrs(Q3270::CharsetAttr));

    setChar(fromCell.getEBCDIC());
}
