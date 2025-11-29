/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QGuiApplication>
#include <QClipboard>
#include <QGraphicsRectItem>
#include <QRegion>

#include <arpa/telnet.h>

#include "Q3270.h"
#include "DisplayScreen.h"

/**
 * @brief   DisplayScreen::DisplayScreen - the 3270 display matrix, representing primary or alternate screens.
 * @param   screen_x - the width of the screen
 * @param   screen_y - the height of the screen
 * @param   cp       - the codepage being used
 * @param   palette  - the colour theme being used
 * 
 * @details DisplayScreen manages the 3270 display matrix. It is responsible for placing characters on
 *          the screen and for managing fields, graphic escape, attributes and so on. 
 *          
 *          The screen is defined as a width x height area. Each cell within that area is CELL_WIDTH by
 *          CELL_HEIGHT. This is a 4:3 ratio akin to the original 3270 screens; scaled by Qt as required.
 *          
 *          DisplayScreen also handles the crosshairs (the ruler, which tracks where the cursor is), and
 *          the rubberband for selecting, copying and pasting sections of the screen.
 */
DisplayScreen::DisplayScreen(int screen_x, int screen_y, CodePage &cp, const Colours *palette)
    : cp(cp)
    , palette(palette)
    , screen_x(screen_x)
    , screen_y(screen_y)
{
    this->setPos(0, 0);

    gridSize_X = CELL_WIDTH;
    gridSize_Y = CELL_HEIGHT;

    setSize(screen_x, screen_y);

    // Default settings
    ruler = Q3270::CrossHair;
    rulerOn = false;
    blinkShow = true;
    cursorShow = true;
    cursorColour = true;
    fontTweak = Q3270::None;

    cursor_pos = 0;

    // Rubberband; QRubberBand can't be used directly on QGraphicsItems
    QPen myRbPen = QPen();
    myRbPen.setWidth(0);
    myRbPen.setBrush(QColor(Qt::yellow));
    myRbPen.setStyle(Qt::DotLine);

    myRb = new QGraphicsRectItem(this);
    myRb->setPen(myRbPen);
    myRb->setZValue(10);
    myRb->hide();

    setFont(QFont("ibm3270", 14));

    // Set up cursor
    cursor.setRect(0, 0, gridSize_X, gridSize_Y);
    cursor.setPos(0, 0);
    cursor.setBrush(Qt::lightGray);
    cursor.setOpacity(0.5);
    cursor.setPen(Qt::NoPen);
    cursor.setParentItem(this);
    cursor.setZValue(4);

    crosshair_X.setPen(QPen(Qt::white, 0));
    crosshair_Y.setPen(QPen(Qt::white, 0));

    crosshair_X.setZValue(5);
    crosshair_Y.setZValue(5);

    crosshair_X.setParentItem(this);
    crosshair_Y.setParentItem(this);

    crosshair_X.hide();
    crosshair_Y.hide();
}

/**
 * @brief   DisplayScreen::~DisplayScreen
 * 
 * @details Not sure we need this!
 */
DisplayScreen::~DisplayScreen()
{
}

QRectF DisplayScreen::boundingRect() const
{
    return QRectF(0, 0, screen_x * CELL_WIDTH, screen_y * CELL_HEIGHT);
}


/**
 * @brief   DisplayScreen::width - return the width of the screen
 * @return  the width passed to the constructor
 * 
 * @details width is called to extract the horizontal size of the screen. It is used by the
 *          Read Partition (Query) structure field which responds to the host with the capabilities
 *          of the terminal. 
 */
int DisplayScreen::width() const
{
    return screen_x;
}

/**
 * @brief   DisplayScreen::height - return the height the screen
 * @return  the height passed to the constructor
 * 
 * @details width is called to extract the vertical size of the screen. It is used by the
 *          Read Partition (Query) structure field which responds to the host with the capabilities
 *          of the terminal. 
 */
int DisplayScreen::height() const
{
    return screen_y;
}

void DisplayScreen::setSize(const int x, const int y)
{
    screen_x = x;
    screen_y = y;

    screenPos_max = x * y;

    // Set up crosshairs
    crosshair_X.setLine(0, 0, 0, screen_y * gridSize_Y);
    crosshair_Y.setLine(0, 0, screen_x * gridSize_X, 0);

    // Build 3270 display matrix
    cells.resize(screenPos_max);

    // Clear matrix and set initial attributes
    clear();
}

/**
 * @brief   DisplayScreen::setFont - change the font on the screen
 * @param   font - the new font
 * 
 * @details setFont is called when the user has changed the font that is used to display the
 *          characters on the screen. Each cell is updated with the new font. 
 */
void DisplayScreen::setFont(const QFont &font)
{
    this->font = font;
    updateFontMetrics();
}

/**
 * @brief   DisplayScreen::setFontTweak - change the way zero is displayed
 * @param   f - how a zero is modified on screen
 *
 * @details setFontTweak changes the way a zero is displayed.
 */
void DisplayScreen::setFontTweak(const Q3270::FontTweak f)
{
    this->fontTweak = f;
    updateFontMetrics();
}

/**
 * @brief   DisplayScreen::updateFontMetrics - update the font metrics for zero overlays
 *
 * @details The position of a slash or a dot for a font tweak zero overlay will vary depending
 *          on the font. This routine calculates new positions.
 *
 * @note    The maths in this routine was devised by Copilot.
 */
void DisplayScreen::updateFontMetrics()
{
    QFontMetrics fm(font);
    int w = fm.horizontalAdvance(QChar('0')); // glyph width
    int h = fm.height();                      // glyph height

    // Dot at glyph center
    dotOffset = QPoint(gridSize_X/2, gridSize_Y/2);
    dotRadius = qMax(1, (int)(gridSize_Y / 8));  // scales with cell height

    // Dot radius can also scale
    dotRadius = qMax(1, h / 12);

    // Slash calculations
    int gh = fm.height();

    w = gridSize_X;
    h = gridSize_Y;

    QPointF center(w/2.0, h/2.0);

    // Slash length proportional to font height, but clamped to cell diagonal
    double L = std::min(gh * 0.6, std::sqrt(w*w + h*h) * 0.9);

    // 45° angle
    double dx = std::cos(M_PI/3) * (L/2.0);
    double dy = std::sin(M_PI/3) * (L/2.0);

    slashStart = QPoint(qRound(center.x() + dx), qRound(center.y() - dy));
    slashEnd   = QPoint(qRound(center.x() - dx), qRound(center.y() + dy));
}

/**
 * @brief   DisplayScreen::resetColours - set the colours of each cell
 * 
 * @details resetColours updates each cell with the modified colour palette. This is called
 *          when the user has changed the colour palette. 
 */
void DisplayScreen::resetColours()
{
    update();
}


/**
 * @brief   DisplayScreen::clear - clear the screen
 * 
 * @details clear is called at first 'power on', when the EW or EWA commands are received, and also
 *          when the user presses the Clear button. 
 *          
 *          All fields are wiped, all attributes are reset and the display is filled with nulls. 
 */
void DisplayScreen::clear()
{
    for(int i = 0; i < screenPos_max; i++)
    {
        cells[i].setFieldStart(false);

        cells[i].setNumeric(false);
        cells[i].setMDT(false);
        cells[i].setProtected(false);
        cells[i].setDisplay(true);
        cells[i].setPenSelect(false);
        cells[i].setIntensify(false);

        cells[i].setExtended(false);
        cells[i].setUnderscore(false);
        cells[i].setReverse(false);
        cells[i].setBlink(false);

        cells[i].setColour(Q3270::Green);

        cells[i].setChar(IBM3270_CHAR_NULL);
        cells[i].setField(nullptr);
    }
    resetCharAttr();

    geActive = false;
    unformatted = true;

    setCursor(0);
}

/**
 * @brief   DisplayScreen::setChar - place a character on the screen
 * @param   pos    - the cell in which to place the character
 * @param   c      - the character to be set
 * @param   fromKB - whether the character was generated by the keyboard
 * 
 * @details setChar is called whenever a character is to be placed on the screen. This can be either from
 *          the incoming 3270 data stream, or from the keyboard. Whichever method generated the call, 
 *          any field start is removed (this can only happen from the datastream, because field starts
 *          are protected by nature). Any character attributes previously present in the cell are removed.
 *          
 *          The field containing the character is used to determine the colour of the character, unless
 *          character attributes are in effect.
 */
void DisplayScreen::setChar(int pos, uchar c, bool fromKB)
{

    Cell &thisCell = cells[pos];

    // If we're overlaying a Field Start, the subsequent positions on the screen now have a different
    // field start, so update them accordingly.
    if (thisCell.isFieldStart())
    {
        thisCell.setFieldStart(false);

        Cell *lastField;

        // If this is the first position on the screen, we need the last screen position's field
        lastField = cells[pos == 0 ? screenPos_max - 1 : pos - 1].getField();

        int tmpPos = pos;

        while(!cells[tmpPos % screenPos_max].isFieldStart() && tmpPos < pos + screenPos_max)
        {
            int i1 = tmpPos++ % screenPos_max;
//          qDebug() << "Field at" << pos << "was FieldStart. Updating" << i1 << "as" << lastField;
            cells[i1].setField(lastField);
        }
    }

    Cell *fieldAttr = thisCell.getField();

    // If the field attribute is not set, use the current position
    if (fieldAttr == nullptr)
        fieldAttr = &cells[pos];

    // Set character attribute flags if applicable
    if (useCharAttr)
       applyCharAttributes(pos, fieldAttr);

    // Choose a graphic character if needed
    thisCell.setGraphic(geActive);

    if (!fromKB)
    {
        thisCell.setChar(c);
    }
    else
    {
        thisCell.setChar(cp.getEBCDIC(c));
    }

    geActive = false;

    // Colour
    if (thisCell.hasCharAttrs(Q3270::CharAttr::ColourAttr) && !charAttr.colour_default)
        thisCell.setColour(charAttr.colNum);
    else
        thisCell.setColour(fieldAttr->getColour());

    // Extended attributes
    if (thisCell.hasCharAttrs(Q3270::CharAttr::ExtendedAttr))
    {
        thisCell.setReverse(charAttr.reverse_default   ? fieldAttr->isReverse() : charAttr.reverse);
        thisCell.setUnderscore(charAttr.uscore_default ? fieldAttr->isUScore()  : charAttr.uscore);
        thisCell.setBlink(charAttr.blink_default       ? fieldAttr->isBlink()   : charAttr.blink);
    }
    else
    {
        thisCell.setReverse(fieldAttr->isReverse());
        thisCell.setUnderscore(fieldAttr->isUScore());
        thisCell.setBlink(fieldAttr->isBlink());
    }

    // Maintain blink cells rectangles for blink()
    int row = pos / screen_x;
    int col = pos % screen_x;

    QRect cellRect(col * gridSize_X, row * gridSize_Y, gridSize_X, gridSize_Y);

    if (thisCell.isBlink())
    {
        blinkCells += cellRect;
    }
    else
    {
        blinkCells -= cellRect;
    }

    // If character colour attributes are present, use them instead
//    if (cells[pos].hasCharAttrs(Q3270::ColourAttr) || cells[pos].hasCharAttrs(Q3270::ExtendedAttr))
//        applyCharAttrsOverrides(pos, fieldAttr);
}

/**
 * @brief   DisplayScreen::setCharAttr - set character attributes
 * @param   extendedType  - the character attribute to set
 * @param   extendedValue - the value
 *
 * @details setCharAttr is called when the SA order is encountered in the 3270 data stream.
 *          SA orders set the character attributes for the next characters placed on the screen
 *          until one of the conditions turns them off.
 *
 *          SA orders have two bytes; the attribute type, and the value.
 *
 * @note    There are more character attributes than Q3270 currently supports.
 */
void DisplayScreen::setCharAttr(unsigned char extendedType, unsigned char extendedValue)
{
//    printf("[SetAttribute ");

    switch(extendedType)
    {
        case IBM3270_EXT_DEFAULT:
            charAttr.blink_default = true;
            charAttr.reverse_default = true;
            charAttr.uscore_default = true;
            charAttr.colour_default = true;
//            printf("default");
            break;
        case IBM3270_EXT_HILITE:
            switch(extendedValue)
            {
                case IBM3270_EXT_HI_DEFAULT:
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink   = false;
//                    printf("default");
                    break;
                case IBM3270_EXT_HI_NORMAL:
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink   = false;
//                    printf("normal");
                    break;
                case IBM3270_EXT_HI_BLINK:
                    charAttr.blink   = true;
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink_default = false;
//                    printf("blink");
                    break;
                case IBM3270_EXT_HI_REVERSE:
                    charAttr.blink   = false;
                    charAttr.uscore  = false;
                    charAttr.reverse = true;
                    charAttr.reverse_default = false;
//                    printf("reverse");
                    break;
                case IBM3270_EXT_HI_USCORE:
                    charAttr.blink   = false;
                    charAttr.reverse = false;
                    charAttr.uscore  = true;
                    charAttr.uscore_default = false;
//                    printf("uscore");
                    break;
                default:
                    printf("** Extended Value %02X Not Implemented **", extendedValue);
            }
            break;
        case IBM3270_EXT_FG_COLOUR:
            if (extendedValue == IBM3270_EXT_DEFAULT)
            {
                charAttr.colour_default = true;
//                printf("fg colour default");
            }
            else
            {
                charAttr.colour = palette->colour(Q3270::Colour(extendedValue&7));
                charAttr.colNum = (Q3270::Colour)(extendedValue&7);
                charAttr.colour_default = false;
//                printf("fg colour %s (extendedValue %02X)", colName[charAttr.colNum], extendedValue);
            }
            break;
        case IBM3270_EXT_BG_COLOUR:
            if (extendedValue == IBM3270_EXT_DEFAULT)
            {
                charAttr.colour_default = true;
//                printf("bg colour default");
            }
            else
            {
                charAttr.colour = palette->colour(Q3270::Colour(extendedValue&7));
                charAttr.colNum = (Q3270::Colour)(extendedValue&7);
                charAttr.colour_default = false;
//                printf("bg colour %s", colName[charAttr.colNum]);
            }
            break;
        default:
            printf(" ** Extended Type %02X Not implemented **", extendedType);
    }
//    printf("]");
//    fflush(stdout);

    useCharAttr = true;

}

/**
 * @brief   DisplayScreen::resetCharAttr - set character attributes to default
 *
 * @details Character attributes are reset when another 3270 write command is sent or the
 *          Clear key is pressed.
 */
void DisplayScreen::resetCharAttr()
{
    charAttr.blink_default = true;
    charAttr.reverse_default = true;
    charAttr.uscore_default = true;
    charAttr.colour_default = true;

    useCharAttr = false;
}

/**
 * @brief   DisplayScreen::setGraphicEscape - indicate that the next character is a graphic one
 *
 * @details The 3270 Graphic Escape order means that the next character is selected from the internal
 *          0310 code page.
 */
void DisplayScreen::setGraphicEscape()
{
    geActive = true;
}

/**
 * @brief   DisplayScreen::setField - Start Field or Start Field Extended
 * @param   pos - the position on screen
 * @param   c   - the field attribute byte that controls protected, numeric and others
 * @param   sfe - true for a Start Field Extended
 *
 * @details setField marks the beginning of a 3270 field. The Field Attribute byte takes the
 *          following form:
 *
 *          Bit | Function
 *          --- | --------
 *          0,1 | These two bits combine to make the field attribute a valid EBCDIC character
 *            2 | Field is protected if set to 1
 *            3 | Field is numeric if set to 1
 *          4,5 | 00 - Display, non-light pen detectable
 *            ^ | 01 - Display, light pen detectable
 *            ^ | 10 - Intensified, light pen detectable
 *            ^ | 11 - Non-display, non light pen detectable
 *            6 | Reserved. Must be 0.
 *            7 | MDT flag. Set when a field is modified
 *
 *          setField characters are always displayed as nulls. When a Start Field (or SFE) is
 *          encountered, the characters following the field are modified to reflect the field
 *          attribute via the cascadeAttrs function.
 */
void DisplayScreen::setField(int pos, unsigned char c, bool sfe)
{
    // At least one field is defined
    unformatted = false;

    // Set field attribute flags
    bool prot   = (c>>5) & 1;
    bool num    = (c>>4) & 1;
    bool disp   = ((c>>2) & 3) != 3;
    bool pensel = (( c >> 2) & 3) == 2 || (( c >> 2) & 3) == 1;
    bool intens = ((c >> 2) & 3) == 2;
    bool mdt    = c & 1;

    cells[pos].setProtected(prot);
    cells[pos].setNumeric(num);
    cells[pos].setDisplay(disp);
    cells[pos].setPenSelect(pensel);
    cells[pos].setIntensify(intens);
    cells[pos].setMDT(mdt);

    cells[pos].setExtended(sfe);

    cells[pos].setFieldStart(true);

    // Fields are set to 0x00
    cells[pos].setChar(IBM3270_CHAR_NULL);

/*  if (pos == screenPos_max - 1)
    {
        qDebug() << "I'm here";
    }
*/
    cascadeAttrs(pos);
}

/**
 * @brief   DisplayScreen::cascadeAttrs - cascade a field attribute to the cells in the field
 * @param   pos - the position of the field
 *
 * @details When a field is set on the display, all the character cells following it inherit the field
 *          attributes. This routine copies the field attributes to the cells in the field until the next
 *          field start.
 */
void DisplayScreen::cascadeAttrs(int pos)
{
        int endPos = pos + screenPos_max;

        int i = pos + 1;
        while(i < endPos && !(cells[i % screenPos_max].isFieldStart()))
        {
            int offset = i++ % screenPos_max;
            cells[offset].setField(&cells[pos]);
        }
}

/**
 * @brief   DisplayScreen::resetExtended - reset extended attributes ready for a Start Field Extended
 * @param   pos - the field position
 *
 * @details When a SFE order is encountered, there is no way to know which attributes will be set in the
 *          extended attribute pairs that follow the SFE, so this routine makes sure that any existing
 *          attribute settings are cleared before processing the pairs.
 */
void DisplayScreen::resetExtended(int pos)
{
    resetExtendedHilite(pos);

    cells[pos].setColour(Q3270::Blue);

    cells[pos].setDisplay(true);
    cells[pos].setNumeric(false);
    cells[pos].setMDT(false);
    cells[pos].setPenSelect(false);
    // FIXME: is this right that setProtected is commented out? Protecton comes from the field attr
    // cell.at(pos)->setProtected(false);
}

/**
 * @brief   DisplayScreen::resetExtendedHilite - reset the extended highlighting
 * @param   pos - the position on screen
 *
 * @details Extended attribute pairs may set the highlighting to default or reset it. In both
 *          cases, this routine is used to switch off any existing highlighting.
 */
void DisplayScreen::resetExtendedHilite(int pos)
{
    cells[pos].setUnderscore(false);
    cells[pos].setBlink(false);
    cells[pos].setReverse(false);
}

/**
 * @brief   DisplayScreen::setExtendedColour - set the extended colour attributes
 * @param   pos        - screen position
 * @param   foreground - foreground or background (true for foreground)
 * @param   c          - the colour to set
 *
 * @details When the extended attribute pair is a set foreground colour or set background colour,
 *          this routine modifies the colour accordingly.
 */
void DisplayScreen::setExtendedColour(int pos, bool foreground, unsigned char c)
{
    //TODO: Invalid colours?
    if (c == IBM3270_EXT_DEFAULT)
    {
        return;
        c = IBM3270_EXT_DEFAULT_COLOR;
    }
    cells[pos].setColour((Q3270::Colour)(c&7));
}

//FIXME: Cell() should do the co-ordination of underscore/blink/reverse, resulting in 2 fewer calls for each

/**
 * @brief   DisplayScreen::setExtendedBlink - switch blink on
 * @param   pos - screen position
 *
 * @details Set the cell at position to blink. Blink, Reverse and Underscore are mutually exclusive.
 */
void DisplayScreen::setExtendedBlink(int pos)
{
    cells[pos].setUnderscore(false);
    cells[pos].setReverse(false);
    cells[pos].setBlink(true);
}

/**
 * @brief   DisplayScreen::setExtendedReverse - switch reverse on
 * @param   pos - screen position
 *
 * @details Set the cell at position to reverse. Blink, Reverse and Underscore are mutually exclusive.
 */
void DisplayScreen::setExtendedReverse(int pos)
{
    cells[pos].setUnderscore(false);
    cells[pos].setBlink(false);
    cells[pos].setReverse(true);
}

/**
 * @brief   DisplayScreen::setExtendedUscore - switch underscore on
 * @param   pos - screen position
 *
 * @details Set the cell at position to underscore. Blink, Reverse and Underscore are mutually exclusive.
 */
void DisplayScreen::setExtendedUscore(int pos)
{
    cells[pos].setBlink(false);
    cells[pos].setReverse(false);
    cells[pos].setUnderscore(true);
}

/**
 * @brief   DisplayScreen::resetMDTs - reset all the MDTs on the screen
 *
 * @details Reset all MDTs in the display; it's probably faster to just loop through the entire buffer
 *          rather than calling findNextField()
 */
void DisplayScreen::resetMDTs()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        if (cells[i].isFieldStart() && cells[i].isMdtOn())
        {
  /*        qDebug() << "Resetting MDT at" << i;*/
            cells[i].setMDT(false);
        }

    }
}

/**
 * @brief   DisplayScreen::insertChar - Inserts or overwrites the character at the specified position
 * @param   c          - character to be inserted
 * @param   insertMode - true for insert, false for overtype
 *
 * @return  true if insert was successful, false if field protected or not enough space for insert mode
 *
 * @details insertChar is used when a character is entered from the keyboard. if Insert mode is on,
 *          the existing characters to the right of the Cell at pos are shifted right by one Cell if there
 *          is a space or a null at the end. The character is then inserted into the space at pos.
 *
 *          If there isn't enough space, insertChar returns false, otherwise it returns true.
 */
bool DisplayScreen::insertChar(unsigned char c, bool insertMode)
{
    if (cells[cursor_pos].isProtected() || cells[cursor_pos].isFieldStart())
    {
        printf("Protected!\n");
        fflush(stdout);
        return false;
    }

    //int thisField = cell.at(cursor_pos)->getField();

    if (insertMode)
    {
        /** TODO:
         *
         *  Insert only works when there is a null character in the field. If the field
         *  contains spaces, they don't count as nulls. The code below searches for the first null
         *  in the field, allowing the insert to happen only if it finds one.
         *
         *  There is some initial code here to check the last character of a field to see
         *  if it's a space or a null (in which case, the insert could succeed).
         *
         *  x3270 has an option for 'blank fill' which allows a space at the end of field
         *  to be lost when inserting characters; otherwise there must be a null.
         *
         *  What happens when there is a null in the middle of a field, but characters to
         *  the right? Theoretically, when an insert operation happens, the characters to
         *  the right of the insert point are moved to occupy the null (which is lost) and the
         *  characters to the right of the null remain in situ. This might be overthinking it!
         *
         *  Perhaps this should be broken into two tests - if the last char is a null, insert;
         *  If the option to 'blank fill' is enabled, if the last char is a space, insert
         *  otherwise it's overflow.
         **/
        int nextField = findNextField(cursor_pos);
//        printf("This Field at: %d,%d, next field at %d,%d - last byte of this field %02X\n", (int)(thisField/screen_x), (int)(thisField-((int)(thisField/screen_x)*screen_x)), (int)(nextField/screen_x), (int)(nextField-((int)(nextField/screen_x)*screen_x)), cell.at(nextField - 1)->getEBCDIC() );
        uchar lastChar = cells[nextField - 1].getEBCDIC();
        if (lastChar != IBM3270_CHAR_NULL && lastChar != IBM3270_CHAR_SPACE)
        {
            // Insert not okay
        }
        int endPos = -1;
        for(int i = cursor_pos; i < (cursor_pos + screenPos_max); i++)
        {
            int offset = i % screenPos_max;
            if (cells[offset].isProtected() || cells[offset].isFieldStart())
            {
                break;
            }
            if (cells[offset].getEBCDIC() == IBM3270_CHAR_NULL)
            {
                endPos = i;
                break;
            }
        }
        if (endPos == -1)
        {
            printf("Overflow!\n");
            fflush(stdout);
            return false;
        }

        for(int fld = endPos; fld > cursor_pos; fld--)
        {
            int offset = fld % screenPos_max;
            int offsetPrev = (fld - 1) % screenPos_max;

            cells[offset].copy(cells[offsetPrev]);
        }
    }

    cells[cursor_pos].setMDT(true);

    setChar(cursor_pos, c, true);

//    cells[cursor_pos].updateCell();

    setCursor((cursor_pos + 1) % screenPos_max);

    if (isAskip(cursor_pos))
    {
        tab(0);
    }

    update();

    return true;
}

/**
 * @brief   DisplayScreen::isAskip - does this Cell have autoskip enabled
 * @param   pos - screen position
 * @return  true if autoskip is on, false otherwise
 *
 * @details isAskip returns a boolean indicating whether the supplied screen position contains
 *          autoskip.
 */
bool DisplayScreen::isAskip(int pos) const
{
    return cells[pos].isAutoSkip();
}

/**
 * @brief   DisplayScreen::isProtected - is this Cell protected?
 * @param   pos - screen position
 * @return  true if protected, false otherwise
 *
 * @details isProtected returns true if the Cell is protected.
 */
bool DisplayScreen::isProtected(int pos) const
{
    return cells[pos].isProtected();
}

/**
 * @brief   DisplayScreen::isFieldStart - is this Cell a Field Start
 * @param   pos - screen position
 * @return  true for a Field Start, false otherwise
 *
 * @details isFieldStart returns true if the Cell is a Field Start
 */
bool DisplayScreen::isFieldStart(int pos) const
{
    return cells[pos].isFieldStart();
}

/**
 * @brief   DisplayScreen::deleteChar - delete the character at the specified position
 * @param   pos - screen position
 *
 * @details deleteChar is used when deleting a character from the keyboard. The characters to the right
 *          of the Cell at pos are shifted left by one character, and a null inserted at the end of the
 *          field.
 */
void DisplayScreen::deleteChar()
{
    if (cells[cursor_pos].isProtected())
    {
        printf("Protected!\n");
        fflush(stdout);
        return;
    }

    int endPos = findNextField(cursor_pos);

    if (endPos < cursor_pos)
    {
        endPos += screenPos_max;
    }

    for(int fld = cursor_pos; fld < endPos - 1 && cells[fld % screenPos_max].getEBCDIC() != IBM3270_CHAR_NULL; fld++)
    {        
        int offset = fld % screenPos_max;
        int offsetNext = (fld + 1) % screenPos_max;

        cells[offset].copy(cells[offsetNext]);
    }

    cells[(endPos - 1) % screenPos_max].setChar(IBM3270_CHAR_NULL);
    cells[cursor_pos].setMDT(true);

    update();
}

/**
 * @brief   DisplayScreen::eraseEOF - clear the Cells starting at pos until the end of the field
 * @param   pos - screen position
 *
 * @details eraseEOF is used when the Erase EOF function is used from the keyboard. The field that Cell
 *          is in is set to null, starting at position pos until the end of the field.
 */
void DisplayScreen::eraseEOF()
{
    int nextField = findNextField(cursor_pos);

    if (nextField < cursor_pos)
    {
        nextField += screenPos_max;
    }

    /* Blank field */
    for(int i = cursor_pos; i < nextField; i++)
    {
        cells[i % screenPos_max].setChar(0x00);
    }

    cells[cursor_pos].setMDT(true);

    update();
}


/**
 * @brief   DisplayScreen::eraseUnprotected - erase unprotected fields between addresses
 * @param   start - screen position
 * @param   end   - screen position
 *
 * @details eraseUnprotected is called when the EUA (Erase Unprotected to Address) order is encountered
 *          in the 3270 data stream. All unprotected fields between the specified positions are
 *          set to nulls.
 */
void DisplayScreen::eraseUnprotected(int start, int end)
{
    if (end < start)
    {
        end += screenPos_max;
    }

    if (cells[start].isProtected())
    {
        start = findNextUnprotectedField(start);
    }

    for(int i = start; i < end; i++)
    {
        if(cells[i].isProtected() || cells[i].isFieldStart())
        {
            i = findNextUnprotectedField(i);
        }
        else
        {
                cells[i].setChar(IBM3270_CHAR_SPACE);
        }
    }
}

/**
 * @brief   DisplayScreen::processAID - process an attention key
 * @param   aid       - the key
 * @param   shortRead - true for short read, false for normal
 *
 * @details Process an attention key. If the key was a short read key (like CLEAR, PA1 etc) then no
 *          fields are returned to the host.
 */
void DisplayScreen::processAID(int aid, bool shortRead)
{
    QByteArray respBuffer = QByteArray();

    respBuffer.append(aid);

    lastAID = aid;

    if (!shortRead)
    {
        addPosToBuffer(respBuffer, cursor_pos);
        getModifiedFields(respBuffer);
    }

    if (aid == IBM3270_AID_CLEAR)
    {
        setCursor(0);
        clear();
    }

    emit bufferReady(respBuffer);
}

/**
 * @brief   ProcessDataStream::interruptProcess - interrupt the current process
 *
 * @details This is processing for ATTN
 */
void DisplayScreen::interruptProcess()
{
    QByteArray b = QByteArray();

    b.append((uchar) IAC);
    b.append((uchar) IP);

    emit bufferReady(b);
}

/**
 * @brief   DisplayScreen::blink - blink the display
 *
 * @details Called by a timer in Terminal to blink any characters that have blink enabled
 */
void DisplayScreen::blink()
{
    blinkShow = !blinkShow;
    for(QRectF r : blinkCells)
    {
        update(r);
    }
}

/**
 * @brief   DisplayScreen::cursorBlink - blink the cursor
 *
 * @details Called by a timer in Terminal to blink the cursor
 */
void DisplayScreen::cursorBlink()
{
    if (cursorShow)
    {
        cursor.hide();
    }
    else
    {
        cursor.show();
    }

    cursorShow = !cursorShow;
}

/**
 * @brief   DisplayScreen::findField - find the field that contains Cell at pos
 * @param   pos - screen position
 * @return  Field Start of pos
 *
 * @details Returns the position on screen of the Field Start for the Cell at pos. Used to determine
 *          the field attributes for a given Cell on screen. If it doesn't find a Field Start, return
 *          the original position.
 */
int DisplayScreen::findField(int pos)
{
//    qDebug() << "Searching for field for" << pos;
    int endPos = pos - screenPos_max;
    int offset = pos;

    for (int i = pos; i > endPos ; i--)
    {
        if (cells[offset].isFieldStart())
        {
            return offset;
        }
        if (--offset < 0)
        {
            offset = screenPos_max - 1;
        }
    }
    return pos;
}

/**
 * @brief   DisplayScreen::findNextField - find the next field for Cell pos
 * @param   pos - screen position
 * @return  Field Start of the next field after pos
 *
 * @details Returns the position on screen of the next Field Start after the Cell at pos. Used to determine
 *          the end of the field (the next Field Start is the end of the previous field). Returns the
 *          original position if no next field.
 */
int DisplayScreen::findNextField(int pos)
{
    int tmpPos;

    if(cells[pos].isFieldStart())
    {
        pos++;
    }

    for(int i = pos; i < (pos + screenPos_max); i++)
    {
        tmpPos = i % screenPos_max;
        if (cells[tmpPos].isFieldStart())
        {
            return tmpPos;
        }
    }
    return pos;
}

/**
 * @brief   DisplayScreen::findNextUnprotectedField - find the next unprotected field
 * @param   pos - screen position
 * @return  Field Start of the next unprotected field
 *
 * @details Find the next field that is unprotected. This incorporates two field start attributes next
 *          to each other - field start attributes are protected, so with two adjacent Field Starts,
 *          the first cannot be an unprotected field. Used by tab, home, and the PT order.
 */
int DisplayScreen::findNextUnprotectedField(int pos)
{
    int tmpPos;
    int tmpNxt;
    for(int i = pos; i < (pos + screenPos_max); i++)
    {
        // Check this position for unprotected and fieldStart and check the next position for
        // fieldStart - an unprotected field cannot start where two fieldStarts are adajacent
        tmpPos = i % screenPos_max;
        tmpNxt = (i + 1) % screenPos_max;
        if (cells[tmpPos].isFieldStart() && !cells[tmpPos].isProtected() && !cells[tmpNxt].isFieldStart())
        {
            return tmpPos;
        }
    }
    printf("No unprotected field found: start = %d, end = %d\n", pos, pos + screenPos_max);
    fflush(stdout);
    return 0;
}

/**
 * @brief   DisplayScreen::findPrevUnprotectedField - find the previous unprotected field
 * @param   pos - screen position
 * @return  Field Start of the previous unprotected field.
 *
 * @details Find the previous field that is unprotected. This incorporates two field start attributes
 *          next to each other - field start attributes are protected, so with two adjacent Field Starts,
 *          the first cannot be an unprotected field.
 */
int DisplayScreen::findPrevUnprotectedField(int pos)
{
    int tmpPos;
    int tmpNxt;

    int endPos = pos - screenPos_max;

    for(int i = pos - 2; i > endPos; i--)
    {
        tmpPos = i;
        if (tmpPos < 0)
        {
            tmpPos = screenPos_max + i;
        }
        // Check this position for unprotected and fieldStart and check the next position for
        // fieldStart - an unprotected field cannot start where two fieldStarts are adajacent
        // As we're searching backwards, providing the next position isn't a fieldStart, we're good.
        tmpNxt = (tmpPos + 1) % screenPos_max;
        if (cells[tmpPos].isFieldStart() && !cells[tmpPos].isProtected() && !cells[tmpNxt].isFieldStart())
        {
            return tmpPos;
        }
    }
    printf("No unprotected field found: start = %d, end = %d\n", pos, pos + screenPos_max);
    fflush(stdout);
    return pos - 1;
}


/**
 *  @brief   DisplayScreen::getModifiedFields - extract all modified fields from the screen
 *  @param   buffer - address of a QByteArray to which the modified fields are appended
 *
 *  @details Locate all modified fields (MDT tags are set) and add them to the provided buffer.
 */
void DisplayScreen::getModifiedFields(QByteArray &buffer)
{
    for(int i = 0; i < screenPos_max; i++)
    {
        if (!unformatted)
        {
            if (!cells[i].isProtected())
            {
                if (cells[i].isFieldStart())
                {
                    qDebug() << "Input field found at " << i << "MDT is" << cells[i].isMdtOn();
                    // This assumes that where two fields are adajcent to each other, the first cannot have MDT set
                    if (cells[i].isMdtOn())
                    {
                        buffer.append(IBM3270_SBA);

                        int nextPos = (i + 1) % screenPos_max;

                        addPosToBuffer(buffer, nextPos);

                        do
                        {
                            uchar b = cells[nextPos++].getEBCDIC();
                            if (b != IBM3270_CHAR_NULL)
                            {
                                buffer.append(b);
                            }
                            nextPos = nextPos % screenPos_max;
                        }
                        while(!cells[nextPos].isFieldStart());
                    }
                }
            }
        }
        else
        {
            uchar b = cells[i].getEBCDIC();
            if (b != IBM3270_CHAR_NULL)
            {
                buffer.append(b);
            }

        }
    }
}

/**
 * @brief   DisplayScreen::addPosToBuffer - insert 'pos' into 'buffer' as two bytes, doubling 0xFF if needed.
 * @param   buffer - QByteArray to append pos into
 * @param   pos    - screen position
 *
 * @details Adds the screen position pos into the buffer. The 3270 datastream uses 0xFF as an control byte
 *          so any actual 0xFF bytes are doubled - a 0xFF 0xFF sequence in the stream indicates a single
 *          0xFF byte.
 */
void DisplayScreen::addPosToBuffer(QByteArray &buffer, int pos)
{
    int byte1;
    int byte2;

    if (screenPos_max < 4096) // 12 bit
    {
        byte1 = twelveBitBufferAddress[(pos>>6) & 0x3F];
        byte2 = twelveBitBufferAddress[(pos & 0x3F)];
    }
    else if (screenPos_max < 16384) // 14 bit
    {
        byte1 = (pos>>8) & 0x3F;
        byte2 = pos & 0xFF;
    }
    else // 16 bit
    {
        byte1 = (pos>>8) & 0xFF;
        byte2 = pos & 0xFF;
    }

    buffer.append(byte1);

    if (byte1 == 0xFF)
    {
        buffer.append(0xFF);
    }

    buffer.append(byte2);

    if (byte2 == 0xFF)
    {
        buffer.append(0xFF);
    }
}

/**
 * @brief   DisplayScreen::dumpFields - debug routine to print out all fields
 *
 * @details Used to debug where fields are on the screen.
 */
void DisplayScreen::dumpFields()
{
    QString line = "";
    for (int i = 0; i < screen_x; i++)
    {
        line.append(QString("%1").arg(i%10));
    }

    qDebug() << "     " << line;

    for(int i = 0; i < screen_y; i++)
    {
        line = "";
        for (int j = 0; j < screen_x; j++)
        {
            int tmppos = i * screen_x + j;

            if (cells[tmppos].isFieldStart())
                if (cells[tmppos].isMdtOn())
                    line.append("F");
                else
                    line.append("f");
            else if (cells[tmppos].getField()->isFieldStart())
                line.append(".");
            else
                line.append("X");
        }
        qDebug() << QString("%1").arg(i, 3) <<  line;
    }
}

/**
 * @brief   DisplayScreen::dumpDisplay - print out a debug replication of the screen
 *
 * @details Used to debug screen layout
 */
void DisplayScreen::dumpDisplay()
{
    qDebug().noquote() << "---- SCREEN ----";

    QString ascii;
    QString hexline;

    for (int i = 0; i < screenPos_max; i++)
    {
        if (i % screen_x == 0 && i > 0)
        {
            qDebug().noquote() << hexline << "|" << ascii.toLatin1().data() << "|";
            hexline = "";
            ascii = "";
        }
        ascii.append(cp.getUnicodeChar(cells[i].getEBCDIC()));
        hexline.append(QString::asprintf("%02X ", cells[i].getEBCDIC()));
    }

    if (!hexline.isEmpty())
        qDebug().noquote() << hexline << "|" << ascii.toLatin1().data() << "|";

    qDebug().noquote() << "---- SCREEN ----";
}

/**
 * @brief   DisplayScreen::dumpInfo - display information about the Cell at pos
 *
 * @details Used to display information about the Cell at pos. Mapped to Ctrl-I.
 */
void DisplayScreen::dumpInfo()
{
    int y = cursor_pos / screen_x;
    int x = cursor_pos - y * screen_x;

    qDebug().noquote() << QString::asprintf("Cell at %d (%d, %d)", cursor_pos, x, y);
    qDebug().noquote() << "    Character: \"" << cp.getUnicodeChar(cells[cursor_pos].getEBCDIC()) << "\""
                       << " (hex EBCDIC " << Qt::hex << (int) cells[cursor_pos].getEBCDIC()
                       << "ASCII " << Qt::hex << (int) (cp.getUnicodeChar(cells[cursor_pos].getEBCDIC()).length() > 0 ? cp.getUnicodeChar(cells[cursor_pos].getEBCDIC()).at(0).unicode() : 0) << ")";

    qDebug().noquote() << "    Field Attribute: " << cells[cursor_pos].isFieldStart();
    qDebug().noquote() << "        MDT:       " << cells[cursor_pos].isMdtOn();
    qDebug().noquote() << "        Protected: " << cells[cursor_pos].isProtected();
    qDebug().noquote() << "        Numeric:   " << cells[cursor_pos].isNumeric();
    qDebug().noquote() << "        Display:   " << cells[cursor_pos].isDisplay();

    qDebug().noquote() << "    Extended: " << cells[cursor_pos].isExtended();
    qDebug().noquote() << "        Intensify: " << cells[cursor_pos].isIntensify();
    qDebug().noquote() << "        UScore:    " << cells[cursor_pos].isUScore();
    qDebug().noquote() << "        Reverse:   " << cells[cursor_pos].isReverse();
    qDebug().noquote() << "        Blink:     " << cells[cursor_pos].isBlink();

    qDebug().noquote() << "    Character Attributes:";
    qDebug().noquote() << "        Extended: " << cells[cursor_pos].hasCharAttrs(Q3270::ExtendedAttr);
    qDebug().noquote() << "        CharSet:  " << cells[cursor_pos].hasCharAttrs(Q3270::CharsetAttr);
    qDebug().noquote() << "        Colour:   " << cells[cursor_pos].hasCharAttrs(Q3270::ColourAttr);

    qDebug().noquote() << "    Colour:   " << cells[cursor_pos].getColour();
    qDebug().noquote() << "    Graphic:  " << cells[cursor_pos].isGraphic();

    int fieldStart = cells[cursor_pos].getField() - cells.data();
    qDebug().noquote() << "    Field Position: " << fieldStart << "(" << fieldStart / screen_x << "," << (fieldStart - (int) (fieldStart / screen_x) * screen_x) << ")";
}

/**
 * @brief   DisplayScreen::getScreen - place the screen buffer into the 3270 data stream
 * @param   buffer - buffer to add the screen to
 *
 * @details The 3270 command Read Buffer (RB) causes the screen contents to be returned to the
 *          host. getScreen extracts the screen status and adds it to buffer.
 */
void DisplayScreen::getScreen(QByteArray &buffer)
{
    buffer.append(lastAID);

    addPosToBuffer(buffer, cursor_pos);

    dumpDisplay();

    for (int i = 0; i < screenPos_max; i++)
    {
        if (cells[i].isFieldStart())
        {
            buffer.append(IBM3270_SF);
            uchar attr;
            if (cells[i].isDisplay() && !cells[i].isPenSelect())
            {
                attr = 0x00;
            }
            else if (cells[i].isDisplay() && cells[i].isPenSelect())
            {
                attr = 0x01;
            }
            else if(cells[i].isIntensify())
            {
                attr = 0x10;
            }
            else
            {
                attr = 0x11;
            }

            int byte = twelveBitBufferAddress[cells[i].isMdtOn() | attr << 3 | cells[i].isNumeric() << 4 | cells[i].isProtected() << 5];

            buffer.append(byte);

            // Double up 0xFF bytes
            if (byte == 0xFF)
            {
                buffer.append(byte);
            }
        }
        else
        {
            buffer.append(cells[i].getEBCDIC());
        }
    }
}

/**
 * @brief   DisplayScreen::applyCharAttributes - apply the character attributes to the cell
 * @param   pos       - screen position
 * @param   fieldAttr - field attribute
 *
 * @details Apply the character attributes to the cell at pos. This is used when the datastream
 *          selected a different colour for the specified cell.
 */
void DisplayScreen::applyCharAttributes(int pos, Cell *field)
{
    if (!charAttr.colour_default)
        cells[pos].setCharAttrs(Q3270::ColourAttr, true);
    else
        cells[pos].setColour(field->getColour());

    if (!charAttr.blink_default)
        cells[pos].setCharAttrs(Q3270::ExtendedAttr, true);
    else
        cells[pos].setBlink(field->isBlink());

    if (!charAttr.reverse_default)
        cells[pos].setCharAttrs(Q3270::ExtendedAttr, true);
    else
        cells[pos].setReverse(field->isReverse());

    if (!charAttr.uscore_default)
        cells[pos].setCharAttrs(Q3270::ExtendedAttr, true);
    else
        cells[pos].setUnderscore(field->isUScore());
}

void DisplayScreen::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
    p->fillRect(boundingRect(), QColor(Qt::black));

    p->setFont(font);

    QPen us;

    us.setWidth(0);
    us.setCosmetic(true);

    for (int r = 0; r < screen_y; ++r)
    {
        for (int c = 0; c < screen_x; ++c)
        {
            const Cell &cs = cells[r * screen_x + c];

            QRectF rect(c * gridSize_X, r * gridSize_Y, gridSize_X, gridSize_Y);

            QColor fg = palette->colour(cs.getColour());
            QColor bg = QColor(Qt::black);

            // reverse
            if (cs.isReverse())
            {
                std::swap(fg, bg);
                p->fillRect(rect, bg);
            }

            // blink
            if (!(cs.isBlink() && !blinkShow))
            {

                // glyph
                if (!(cs.getEBCDIC() == IBM3270_CHAR_NULL) && cs.isDisplay() && !cs.isFieldStart())
                {
                    p->setPen(fg);
                    if (!cs.isGraphic())
                    {
                        p->drawText(rect, Qt::AlignCenter, cp.getUnicodeChar(cs.getEBCDIC()));
                        if (cs.getEBCDIC() == IBM3270_CHAR_ZERO)
                        {
                            // Slash/dot overlay
                            p->save();
                            p->setRenderHint(QPainter::Antialiasing, true);
                            switch(fontTweak)
                            {
                                case Q3270::None:
                                    break;
                                case Q3270::ZeroDot:
                                    p->setBrush(fg);
                                    p->setPen(Qt::NoPen);
                                    p->drawEllipse(rect.topLeft() + dotOffset, dotRadius, dotRadius);
                                    break;
                                case Q3270::ZeroSlash:
                                    p->setBrush(Qt::NoBrush);   // no fill needed
                                    p->setPen(fg);              // stroke colour
                                    p->drawLine(rect.topLeft() + slashStart,
                                                rect.topLeft() + slashEnd);
                                    break;
                            }
                            p->restore();
                        }
                    }
                    else
                    {
                        p->drawText(rect, Qt::AlignCenter, cp.getUnicodeGraphicChar(cs.getEBCDIC()));
                    }
                }
            }

            // underscore
            if (cs.isUScore() && !cs.isFieldStart() && cs.isDisplay())
            {
                p->setPen(fg);
                p->drawLine(QPoint(rect.left(), rect.bottom() - 1), QPoint(rect.right(), rect.bottom() - 1));
            }
        }
    }
/*
    QPen pen(QColor(128,128,128,64));
    pen.setWidth(0);
    p->setPen(pen);

    for (int c = 0; c <= screen_x; ++c) {
        qreal x = c * gridSize_X + 0.5;
        p->drawLine(x, 0, x, screen_y * gridSize_Y);
    }
    for (int r = 0; r <= screen_y; ++r) {
        qreal y = r * gridSize_Y + 0.5;
        p->drawLine(0, y, screen_x * gridSize_X, y);
    }
*/
}
