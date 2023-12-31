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
 * This class is where the colours, visibility and reverse status are set.
 *
 * @param x_pos     -the across position in the 3270 display of this cell
 * @param y_pos    - the down position in the 3270 display of this cell
 * @param x        - the width of the cell in the parent QGraphicRectItem terms
 * @param y        - the height of the cell in the parent QGraphicsRectItem terms
 * @param &cp      - the codepage of the display (there is one codepage for all the display)
 * @param &palette - the colour palette of the display
 * @param *parent  - the parent QGraphicsRectItem (which owns all the Cells)
 * @param *scene   - the scene object
 *
 * @details
 * The scene object is the parent for the overall display, each character and the underscore.
 *
 * Underscores are owned by the scene, as is the character itself (the glyph). The Cell is owned
 * by the 3270 display (24x80 etc) so that the Cell (a QGraphicsRectItem) can have inverted colours
 * which fill the rectangle.
 *
 * The underscore is set at 98% of the height. It is always a single pixel high and is logically
 * the highest layer.
 *
 * Expensive Qt operations (such as changing colours) are only performed once when the incoming
 * datastream has been processed.
 *
 * This is because some 3270 orders cause the display to repeatedly do things - for example, the
 * first "Start Field" order causes all the cells on the screen to have the same value. The next
 * "Start Field" causes all the cells following that until the first "Start Field" is encountered (as
 * the processing wraps around the display). Each "Start Field" therefore processes fewer and fewer
 * locations as the screen fills up with fields.
 *
 * Changing colours each and every time causes noticable delays so instead, when processing the
 * datastream, the updated colour is recorded and a flag set to say that something in this cell
 * has changed.
 *
 * When the datastream is complete, a single call performs the Qt call to updates the colours,
 * reverse, display etc
 *
 * Each Cell contains attributes that are only relevant in particular situations - protection, display, numeric and
 * others can only be set by a Field Start, but given that any cell on the screen can be a field, all cells need to
 * have that potential setting.
 */

Cell::Cell(qreal x_pos, qreal y_pos, qreal x, qreal y, CodePage &cp, ColourTheme::Colours &palette, QGraphicsItem *parent, QGraphicsScene *scene) : xsize(x), ysize(y), palette(palette), cp(cp)
{
    QPen p;
    p.setStyle(Qt::NoPen);
    p.setCosmetic(true);

    this->setParentItem(parent);

    this->setRect(0, 0, x, y);
    this->setPen(p);
    this->setZValue(0);
    this->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    this->setPos(x_pos, y_pos);

    glyph.setBrush(QColor(Qt::white));
    glyph.setParentItem(this);
    glyph.setPos(x_pos, y_pos);
    glyph.setZValue(2);

    underscore.setLine(0, 0, x, 0);
    underscore.setPos(x_pos, y_pos + (y *.98));
    underscore.setVisible(false);
    underscore.setZValue(3);

    scene->addItem(&glyph);
    scene->addItem(&underscore);

    changed = false;
}

/**
 * @brief   Cell::boundingRect
 * @return  the rectangle comprising the cell.
 *
 * @details Not sure this is needed any more.
 */
QRectF Cell::boundingRect() const
{
    return QRectF(0, 0, xsize, ysize);
}

/**
 * @brief   Cell::setUnderscore - switch underscore on or off
 * @param   onoff - true to show the underscore, false to hide it
 *
 * @details setUnderscore shows or hides the underscore. This function doesn't actually
 *          do the swtiching on or off, it only sets the flag. updateCell() performs the
 *          Qt operation to show/hide the graphic.
 */
void Cell::setUnderscore(bool onoff)
{
    uscore = onoff;
    changed = true;
}

/**
 * @brief   Cell::setChar - set the cell to the character specified
 * @param   ebcdic - the EBCDIC character code to be set
 *
 * @details setChar() is what makes the character on the screen visible. If the 'non-display'
 *          flag is set, the character is shown as a space, as are nulls, even though the
 *          underlying character is still stored. If the 'graphic escape' flag is set, the character
 *          is taken from the code page 0310, which is used for things like dialog box borders in ISPF.
 */
void Cell::setChar(uchar ebcdic)
{
    // Characters that are nulls are set to blank on screen, as are non-display characters.
    // Obviously the underlying character value is stored still.
    if (ebcdic == 0x00 || !isDisplay())
    {
        glyph.setText(" ");
    }
    else
    {
        if (!graphic)
        {
            glyph.setText(cp.getUnicodeChar(ebcdic));
        }
        else
        {
            glyph.setText(cp.getUnicodeGraphicChar(ebcdic));
        }
    }

    this->ebcdic = ebcdic;
}

/**
 * @brief   Cell::refreshCodePage - reset the character displayed based on a (maybe) changed codepage.
 *
 * @details refreshCodePage() simply calls setChar() with the current character which will drive the
 *          code page choice again.
 */
void Cell::refreshCodePage()
{
    setChar(ebcdic);
}

/**
 * @brief   Cell::setCharFromKB - set the cell character to that of the ASCII character specified
 * @param   ascii - the ASCII character from the keyboard
 *
 * @details This is the counterpart to setChar() which sets the character to the EBCDIC value. This
 *          function is used when the user types characters at the keyboard.
 *
 * @note    ASCII is used in this context, but in reality it could be any valid character generated from
 *          the keyboard in whatever codepage the keyboard is using.
 */
void Cell::setCharFromKB(uchar ascii)
{
    setChar(cp.getEBCDIC(ascii));
}

/**
 * @brief   Cell::setColour - set the cell to colour specified
 * @param   c - the new colour
 *
 * @details This function sets the colour of the cell to the specified value, but it doesn't actually
 *          do the heavy lifting of calling Qt yet, as potentially any data stream may incur multiple
 *          calls to set the colour of same cell. The Qt part is done in updateCell().
 */
void Cell::setColour(ColourTheme::Colour c)
{
    colNum = c;
    changed = true;
}

/**
 * @brief   Cell::setFieldStart - set the 'Field' flag to show whether this cell is the start of a field
 * @param   fs - true for a field start, false for a normal character.
 *
 * @details setFieldStart() is called when the incoming data stream contains a SF or SFE order; it is also
 *          called when the cell used to be a field start, but that has now been overwritten.
 *
 *          Setting the cell to a Field Start causes underscore, reverse and blinking to be switched off.
 */
void Cell::setFieldStart(bool fs)
{
    fieldStart = fs;

    if (fieldStart)
    {
        setUnderscore(false);
        setReverse(false);
        setBlink(false);
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
void Cell::setNumeric(bool n)
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
void Cell::setGraphic(bool ge)
{
    graphic = ge;
}

/**
 * @brief   Cell::setMDT - switch the MDT flag on or off
 * @param   m - true to turn it on, false to turn it off
 *
 * @details The MDT flag is used to indicate whether the user has modified a particular field.
 */
void Cell::setMDT(bool m)
{
    mdt = m;
}

/**
 * @brief   Cell::setProtected - switch protection on or off for this cell
 * @param   p - true for protected, false for unprotected
 *
 * @details Protected cells cannot be modified by the user. They are intended for text on the screen that
 *          is meant to guide the user (field prompts, menu items and so on).
 */
void Cell::setProtected(bool p)
{
    prot = p;
}

/**
 * @brief   Cell::setDisplay - switch display on or offf for this cell
 * @param   d - true for display, false for non-display
 *
 * @details Non-display fields are used for things like passwords; although the user can enter data into
 *          the cell, it is not shown on screen.
 *
 *          This routine does not change the Qt side; updateCell() does that.
 */
void Cell::setDisplay(bool d)
{
    display = d;
    changed = true;
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
void Cell::setPenSelect(bool p)
{
    pen = p;
}

/**
 * @brief   Cell::setIntensify - set the intensity of the cell
 * @param   i - true for high-intensity, false for low-intensity
 *
 * @details In basic four colour mode, high-intensity fields are displayed in red (unprotected) or white (protected);
 *          low-intensity fields are displayed in blue (protected) or green (unprotected).
 */
void Cell::setIntensify(bool i)
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
void Cell::setExtended(bool e)
{
    extended = e;
}

/**
 * @brief   Cell::setReverse - set the reverse video state of the cell
 * @param   r - true for reverse video, false for normal.
 *
 * @details Reverse video is an extended field attribute or a character attribute. Cells can be
 *          either reversed, underscored or blinking. The co-ordindation of that is done by DisplayScreen.
 *
 *          This routine does not change the Qt colours; that's done by updateCell().
 */
void Cell::setReverse(bool r)
{
    reverse = r;
    changed = true;
}

/**
 * @brief   Cell::setBlink - set the blink status of the character
 * @param   b - true for blink, false for static
 *
 * @details Blink is an extended field attribute or a character attribute. Cells can be
 *          either reversed, underscored or blinking. The co-ordindation of that is done by DisplayScreen.
 */
void Cell::setBlink(bool b)
{
    blink = b;
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
void Cell::setCharAttrs(Cell::CharAttr ca, bool c)
{
    switch(ca)
    {
        case EXTENDED:
            charAttrExtended = c;
            break;
        case COLOUR:
            charAttrColour = c;
            break;
        case CHARSET:
            charAttrCharSet = c;
            break;
        case TRANSPARENCY:
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
bool Cell::hasCharAttrs(Cell::CharAttr ca)
{
    switch(ca)
    {
        case EXTENDED:
            return charAttrExtended;
        case COLOUR:
            return charAttrColour;
        case CHARSET:
            return charAttrCharSet;
        case TRANSPARENCY:
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
 * @details When movning characters around on the screen through the keyboard (ie, through
 *          insert or delete actions), character attributes move with the character.
 */
void Cell::copy(Cell &fromCell)
{
    prot = fromCell.isProtected();
    mdt = fromCell.isMdtOn();
    num = fromCell.isNumeric();
    pen = fromCell.isPenSelect();
    setDisplay(fromCell.isDisplay());

    blink = fromCell.isBlink();

    setColour(fromCell.getColour());
    setUnderscore(fromCell.isUScore());
    setReverse(fromCell.isReverse());

    graphic = fromCell.isGraphic();

    setCharAttrs(EXTENDED, fromCell.hasCharAttrs(EXTENDED));
    setCharAttrs(COLOUR, fromCell.hasCharAttrs(COLOUR));
    setCharAttrs(TRANSPARENCY, fromCell.hasCharAttrs(TRANSPARENCY));
    setCharAttrs(CHARSET, fromCell.hasCharAttrs(CHARSET));

    setChar(fromCell.getEBCDIC());

    updateCell();
}

/**
 * @brief   Cell::setAttrs - set the attributes for a field
 * @param   prot   - protected state
 * @param   mdt    - mdt state
 * @param   num    - numeric state
 * @param   pensel - light pen select state
 * @param   blink  - blink state
 * @param   disp   - display state
 * @param   under  - underscore state
 * @param   rev    - reverse state
 * @param   col    - colour
 *
 * @details setAttrs forms a shortcut to calling all the relevant routines in one go. This routine is used when
 *          setting a Field Start, and cascading all the attributes to the end of the (new) field.
 */
void Cell::setAttrs(bool prot, bool mdt, bool num, bool pensel, bool blink, bool disp, bool under, bool rev, ColourTheme::Colour col)
{
    setProtected(prot);
    setMDT(mdt);
    setNumeric(num);
    setPenSelect(pensel);
    setBlink(blink);
    setDisplay(disp);
    setUnderscore(under);
    setReverse(rev);
    setColour(col);
}

/**
 * @brief   Cell::blinkChar - blink a character
 * @param   blink - the current blink state
 *
 * @details blinkChar is called from DisplayScreen via a timer in Terminal connected to a signal.
 *          At alternate intervals, the character is shown, then hidden.
 */
void Cell::blinkChar(bool blink)
{
    if (blink)
    {
        glyph.setBrush(palette[colNum]);
    }
    else
    {
        glyph.setBrush(palette[ColourTheme::Colour::BLACK]);
    }
}

/**
 * @brief   Cell::setFont - set the font used to display the character
 * @param   f - the font to be used.
 *
 * @details The idea here is to try to make the characters fit into the cell. The largest 3270
            character is the graphic escape which forms a complete cross from top to bottom and left to right.

            This character, used, for example, in dialog boxes and some table type displays, should connect
            to the one above, below, left or right without and gap.

            One drawback is that Qt uses characters from another font if the chosen font does not have
            the character required. This can lead to gappy boxes (ISPF drop-down menus and dialog boxes
            for example).
 */
void Cell::setFont(QFont f)
{
    QFontMetricsF fm = QFontMetrics(f);

    qreal xs;
    qreal ys;

    f.setStyleStrategy( QFont::NoFontMerging);
    f.setStyleStrategy(QFont::NoSubpixelAntialias);

    xs = fm.horizontalAdvance("┼", 1);
    ys = fm.height();

    QTransform fontScale;

    fontScale.scale(xsize / xs, ysize / ys);

    glyph.setTransform(fontScale);

    glyph.setFont(f);
}

/**
 * @brief   Cell::updateCell - update the Qt aspects of the character following other calls to Cell
 *
 * @details When routines in Cell such as setUnderscore and setReverse, are called, they do not make
 *          the updates to Qt immediately because there is a strong possibility that those calls would be
 *          made multiple times during a given incoming datastream. The Qt calls are expensive, so to
 *          improve performance, the call to Qt is only made when the data stream has been processed.
 *
 *          It is also called following an insert or delete operation from the keyboard.
 *
 *          This routine updates Qt based on what has changed for this datastream.
 */
void Cell::updateCell()
{
    if (!changed)
    {
        return;
    }

    changed = false;


    if (!fieldStart)
    {
        underscore.setVisible(uscore);
        if (uscore)
        {
            underscore.setPen(QPen(QColor(palette[colNum]), 0));
        }
        if (reverse)
        {
            glyph.setBrush(palette[ColourTheme::Colour::BLACK]);
            this->setBrush(palette[colNum]);

        }
        else
        {
            glyph.setBrush(palette[colNum]);
            this->setBrush(palette[ColourTheme::Colour::BLACK]);

        }
        if (!display)
        {
            glyph.setVisible(false);
            underscore.setVisible(false);
        }
        else
        {
            glyph.setVisible(true);
            underscore.setVisible(uscore);
        }
    }
    else
    {
        underscore.setVisible(false);
        glyph.setBrush(palette[colNum]);
        this->setBrush(palette[ColourTheme::Colour::BLACK]);
    }
}
