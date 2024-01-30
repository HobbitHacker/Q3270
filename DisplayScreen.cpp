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

#include <QGuiApplication>
#include <QClipboard>

#include <arpa/telnet.h>

#include "Q3270.h"
#include "DisplayScreen.h"

/*
 * 
 * DisplayScreen represents a screen of the 3270 display. The class handles the display matrix.
 *
 * This is created by Terminal.
 */

/**
 * @brief   DisplayScreen::DisplayScreen - the 3270 display matrix, representing primary or alternate screens.
 * @param   screen_x - the width of the screen
 * @param   screen_y - the height of the screen
 * @param   cp       - the codepage being used
 * @param   palette  - the colour theme being used
 * @param   scene    - the parent scene to which this belongs to
 * 
 * @details DisplayScreen manages the 3270 display matrix. It is responsible for placing characters on
 *          the screen and for managing fields, graphic escape, attributes and so on. 
 *          
 *          The screen is defined as a 640x480 QGraphicsRectItem, inside of which is each character, screen_x
 *          by screen_y dimensions. 640x480 was chosen as a reasonable size (3270 screens had a ratio of 4:3)
 *          and which could be automatically scaled by Qt as required. The size of each cell is calculated
 *          as a part of the 640x480 size. 
 *          
 *          DisplayScreen handles the crosshairs (the ruler, which tracks where the cursor is), the
 *          status bar, which shows the cursor position and the insert mode, along with X System.
 *          
 *          It also manages the rubberband for selecting, copying and pasting sections of the screen. 
 */
DisplayScreen::DisplayScreen(int screen_x, int screen_y, CodePage &cp, ColourTheme::Colours &palette, QGraphicsScene *scene) : cp(cp), palette(palette), screen_x(screen_x), screen_y(screen_y)
{

    // 3270 screens are (were) 4:3 ratio, so use a reasonable size that Qt can scale.
    this->setRect(0, 0, 640, 480);
    this->setPos(0, 0);

    gridSize_X = (qreal) 640 / (qreal) screen_x;
    gridSize_Y = (qreal) 480 / (qreal) screen_y;

    screenPos_max = screen_x * screen_y;

    // Default settings
    ruler = Q3270_RULER_CROSSHAIR;
    rulerOn = false;
    blinkShow = true;
    cursorShow = true;
    cursorColour = true;

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

    // Build 3270 display matrix

    cell.resize(screenPos_max);

    for(int y = 0; y < screen_y; y++)
    {
        qreal y_pos = (qreal) y * gridSize_Y;

        for(int x = 0; x < screen_x; x++)
        {
            int pos = x + (y * screen_x);

            qreal x_pos = (qreal) x * gridSize_X;

            cell.replace(pos, new Cell(x_pos, y_pos, gridSize_X, gridSize_Y, cp, palette, this, scene));
        }
    }

    // Set default attributes for initial power-on
    clear();

    setFont(QFont("ibm3270", 14));

    // Set up cursor
    cursor.setRect(cell.at(0)->rect());
    cursor.setPos(0, 0);
    cursor.setBrush(Qt::lightGray);
    cursor.setOpacity(0.5);
    cursor.setPen(Qt::NoPen);
    cursor.setParentItem(this);
    cursor.setZValue(4);

    scene->addItem(&cursor);

    // Set up crosshairs
    crosshair_X.setLine(0, 0, 0, screen_y * gridSize_Y);
    crosshair_Y.setLine(0, 0, screen_x * gridSize_X, 0);

    crosshair_X.setPen(QPen(Qt::white, 0));
    crosshair_Y.setPen(QPen(Qt::white, 0));

    crosshair_X.setZValue(5);
    crosshair_Y.setZValue(5);

    crosshair_X.setParentItem(this);
    crosshair_Y.setParentItem(this);

    crosshair_X.hide();
    crosshair_Y.hide();

    // Build status bar
    statusBar.setLine(0, 0, screen_x * gridSize_X, 0);
    statusBar.setPos(0, 481);
    statusBar.setPen(QPen(QColor(0x80, 0x80, 0xFF), 0));

    QFont statusBarText = QFont("ibm3270");
    statusBarText.setPixelSize(8);

    // Connect status at 0% across
    statusConnect.setText("4-A");
    statusConnect.setPos(0, 481);
    statusConnect.setFont(statusBarText);
    statusConnect.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    // XSystem 20% across status bar
    statusXSystem.setText("");
    statusXSystem.setPos(gridSize_X * (screen_x * .20), 481);
    statusXSystem.setFont(statusBarText);
    statusXSystem.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    // Insert 50% across status bar
    statusInsert.setText("");
    statusInsert.setPos(gridSize_X * (screen_x * .50), 481);
    statusInsert.setFont(statusBarText);
    statusInsert.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    // Cursor 75% across status bar
    statusCursor.setText("");
    statusCursor.setPos(gridSize_X * (screen_x * .75), 481);
    statusCursor.setFont(statusBarText);
    statusCursor.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    scene->addItem(&statusBar);
    scene->addItem(&statusConnect);
    scene->addItem(&statusXSystem);
    scene->addItem(&statusCursor);
    scene->addItem(&statusInsert);

    scene->addItem(this);
}

/**
 * @brief   DisplayScreen::~DisplayScreen
 * 
 * @details Not sure we need this!
 */
DisplayScreen::~DisplayScreen()
{
}

/**
 * @brief   DisplayScreen::width - return the width of the screen
 * @return  the width passed to the constructor
 * 
 * @details width is called to extract the horizontal size of the screen. It is used by the
 *          Read Partition (Query) structure field which responds to the host with the capabilities
 *          of the terminal. 
 */
int DisplayScreen::width()
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
int DisplayScreen::height()
{
    return screen_y;
}

/**
 * @brief   DisplayScreen::gridWidth - return the width of a cell on the screen
 * @return  the width of the character cell
 * 
 * @details width is called to extract the horizontal cell size. It is used by the
 *          Read Partition (Query) structure field which responds to the host with the capabilities
 *          of the terminal. 
 *          
 *          The cell size is calculated as 640 / screen_x.
 */
qreal DisplayScreen::gridWidth()
{
    return gridSize_X;
}

/**
 * @brief   DisplayScreen::gridHeight - return the height of a cell on the screen
 * @return  the height of the character cell
 * 
 * @details width is called to extract the vertical cell size. It is used by the
 *          Read Partition (Query) structure field which responds to the host with the capabilities
 *          of the terminal. 
 *          
 *          The cell size is calculated as 480 / screen_y.
 */
qreal DisplayScreen::gridHeight()
{
    return gridSize_Y;
}

/**
 * @brief   DisplayScreen::setFont - change the font on the screen
 * @param   font - the new font
 * 
 * @details setFont is called when the user has changed the font that is used to display the
 *          characters on the screen. Each cell is updated with the new font. 
 */
void DisplayScreen::setFont(QFont font)
{
    for (int i = 0; i < screenPos_max; i++)
    {
        cell.at(i)->setFont(QFont(font));
    }
}

/**
 * @brief   DisplayScreen::setCodePage - change the codepage used to show the characters on screen
 * 
 * @details The codepage is shared across Q3270, so when it is changed, each cell just needs to be
 *          updated with the new codepage. 
 */
void DisplayScreen::setCodePage()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        cell.at(i)->refreshCodePage();
    }
}

/**
 * @brief   DisplayScreen::resetColours - set the colours of each cell
 * 
 * @details resetColours updates each cell with the modified colour palette. This is called
 *          when the user has changed the colour palette. 
 */
void DisplayScreen::resetColours()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        cell.at(i)->setColour(cell.at(i)->getColour());
    }

    refresh();
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
        cell.at(i)->setFieldStart(false);

        cell.at(i)->setNumeric(false);
        cell.at(i)->setMDT(false);
        cell.at(i)->setProtected(false);
        cell.at(i)->setDisplay(true);
        cell.at(i)->setPenSelect(false);
        cell.at(i)->setIntensify(false);

        cell.at(i)->setExtended(false);
        cell.at(i)->setUnderscore(false);
        cell.at(i)->setReverse(false);
        cell.at(i)->setBlink(false);

        cell.at(i)->setColour(ColourTheme::GREEN);

        cell.at(i)->setChar(0x00);
        cell.at(i)->setField(-1);
    }
    resetCharAttr();

    geActive = false;
    unformatted = true;
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

    int fieldAttr = cell.at(pos)->getField();
    if (fieldAttr == -1)
    {
        fieldAttr = pos;
    }

    if (cell.at(pos)->isFieldStart())
    {
        cell.at(pos)->setFieldStart(false);
        cell.at(pos)->setField(fieldAttr = findField(pos));
    }

    // Reset character attributes for this cell
    cell.at(pos)->resetCharAttrs();

    // Set character attribute flags if applicable
    if (!fromKB && useCharAttr)
    {
        if (!charAttr.colour_default)
        {
            cell.at(pos)->setCharAttrs(Cell::COLOUR, true);
        }
        if (!charAttr.blink_default)
        {
            cell.at(pos)->setCharAttrs(Cell::EXTENDED, true);
        }
        if (!charAttr.reverse_default)
        {
            cell.at(pos)->setCharAttrs(Cell::EXTENDED, true);
        }
        if (!charAttr.uscore_default)
        {
            cell.at(pos)->setCharAttrs(Cell::EXTENDED, true);
        }
    }

    // Non-display comes from field attribute
    cell.at(pos)->setDisplay(cell.at(fieldAttr)->isDisplay());

    // Protected comes from the field attribute
    cell.at(pos)->setProtected(cell.at(fieldAttr)->isProtected());

    // Choose a graphic character if needed
    cell.at(pos)->setGraphic(geActive);

    if (!fromKB)
    {
        cell.at(pos)->setChar(c);
    }
    else
    {
        cell.at(pos)->setCharFromKB(c);
    }

    geActive = false;

    // If character colour attributes are present, use them
    if (cell.at(pos)->hasCharAttrs(Cell::CharAttr::COLOUR))
    {
        // Colour
        if (!charAttr.colour_default)
        {
            cell.at(pos)->setColour(charAttr.colNum);
        }
        else
        {
            cell.at(pos)->setColour(cell.at(fieldAttr)->getColour());
        }
    }
    else
    {
        cell.at(pos)->setColour(cell.at(fieldAttr)->getColour());
    }

    if (cell.at(pos)->hasCharAttrs(Cell::CharAttr::EXTENDED))
    {
        // Reverse video
        if (!charAttr.reverse_default)
        {
            cell.at(pos)->setReverse(charAttr.reverse);
        }
        else
        {
            cell.at(pos)->setReverse(cell.at(fieldAttr)->isReverse());
        }

        // Underscore
        if (!charAttr.uscore_default)
        {
            cell.at(pos)->setUnderscore(charAttr.uscore);
        }
        else
        {
            cell.at(pos)->setUnderscore(cell.at(fieldAttr)->isUScore());
        }

        // Blink
        if (!charAttr.blink_default)
        {
            cell.at(pos)->setBlink(charAttr.blink);
        }
        else
        {
            cell.at(pos)->setBlink(cell.at(fieldAttr)->isBlink());
        }
    }
    else
    {
        cell.at(pos)->setUnderscore(cell.at(fieldAttr)->isUScore());
        cell.at(pos)->setReverse(cell.at(fieldAttr)->isReverse());
        cell.at(pos)->setBlink(cell.at(fieldAttr)->isBlink());
    }
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
                charAttr.colour = palette[(ColourTheme::Colour)(extendedValue&7)];
                charAttr.colNum = (ColourTheme::Colour)(extendedValue&7);
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
                charAttr.colour = palette[(ColourTheme::Colour)(extendedValue&7)];
                charAttr.colNum = (ColourTheme::Colour)(extendedValue&7);
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
    bool prot = (c>>5) & 1;
    bool num = (c>>4) & 1;
    bool disp = ((c>>2) & 3) != 3;
    bool pensel = (( c >> 2) & 3) == 2 || (( c >> 2) & 3) == 1;
    bool intens = ((c >> 2) & 3) == 2;
    bool mdt = c & 1;

    cell.at(pos)->setProtected(prot);
    cell.at(pos)->setNumeric(num);
    cell.at(pos)->setDisplay(disp);
    cell.at(pos)->setPenSelect(pensel);
    cell.at(pos)->setIntensify(intens);
    cell.at(pos)->setMDT(mdt);
    cell.at(pos)->setExtended(sfe);
    cell.at(pos)->setFieldStart(true);

    // Fields are set to 0x00
    cell.at(pos)->setChar(IBM3270_CHAR_NULL);

    // If it's not an Extended Field, set colours accordingly
//    if (!sfe)
//    {
        if (cell.at(pos)->isProtected() && !cell.at(pos)->isIntensify())
        {
            cell.at(pos)->setColour(ColourTheme::PROTECTED_NORMAL);        /* Protected (Blue) */
        }
        else if (cell.at(pos)->isProtected() && cell.at(pos)->isIntensify())
        {
            cell.at(pos)->setColour(ColourTheme::PROTECTED_INTENSIFIED);   /* Protected, Intensified (White) */
        }
        else if (!cell.at(pos)->isProtected() && !cell.at(pos)->isIntensify())
        {
            cell.at(pos)->setColour(ColourTheme::UNPROTECTED_NORMAL);      /* Unprotected (Green) */
        }
        else
        {
            cell.at(pos)->setColour(ColourTheme::UNPROTECTED_INTENSIFIED); /* Unrprotected, Intensified (Red) */
        }
//    }

    cascadeAttrs(pos);
    // Cascade all the attributes until the next field
        /*
    QString attrs;

    if(!cell.at(pos)->isProtected())
    {
        attrs = "unprot,";
    }
    else
    {
        attrs = "prot,";
    }
    if(cell.at(pos)->isIntensify())
    {
        attrs.append("intens,");
    }
    if (cell.at(pos)->isAutoSkip())
    {
        attrs.append("askip,");
    }
    if (!cell.at(pos)->isDisplay())
    {
        attrs.append("nondisp,");
    }
    if (cell.at(pos)->isPenSelect())
    {
        attrs.append("pen,");
    }
    if (cell.at(pos)->isNumeric())
    {
        attrs.append("num,");
    }
    if (cell.at(pos)->isMdtOn())
    {
        attrs.append("mdt,");
    }

    printf("%s", attrs.toLatin1().data());
    fflush(stdout);
*/
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

        bool prot = cell.at(pos)->isProtected();
        bool mdt = cell.at(pos)->isMdtOn();
        bool num = cell.at(pos)->isNumeric();
        bool pensel = cell.at(pos)->isPenSelect();
        bool blink = cell.at(pos)->isBlink();
        bool disp = cell.at(pos)->isDisplay();
        bool under = cell.at(pos)->isUScore();
        bool rev   = cell.at(pos)->isReverse();

        ColourTheme::Colour col = cell.at(pos)->getColour();
        ColourTheme::Colour tmpCol;

        int i = pos + 1;
        while(i < endPos && !(cell.at(i % screenPos_max)->isFieldStart()))
        {
            int offset = i++ % screenPos_max;
            tmpCol = cell[offset]->hasCharAttrs(Cell::COLOUR) ? cell[offset]->getColour() : col;
            cell[offset]->setAttrs(prot, mdt, num, pensel, blink, disp, under, rev, tmpCol);
            cell.at(offset)->setField(pos);
        }
}

/**
 * @brief   DisplayScreen::refresh - process any changes to cells
 *
 * @details When characters are sent to the screen, they may modify colour, underscore, or reverse
 *          settings. These require Qt calls which, when issued repeatedly for the same cell, are
 *          expensive in processing time.
 *
 *          This occurs because a Start Field modifies all following characters until the next
 *          Field Start; the very first field on a screen, therefore, modifies the rest of the screen.
 *          The next field modifies some of those cells again, and so on.
 *
 *          Performing the Qt operations multiple times for the same cell has a negative performance
 *          impact, so the Qt operations are performed just once when the data stream is complete.
 */
void DisplayScreen::refresh()
{
        for (int i = 0; i < screenPos_max; i++)
        {
            cell.at(i)->updateCell();
        }
}

/**
 * @brief   DisplayScreen::resetExtended - reset extended attributes ready for a Start Field Extended
 * @param   pos - the field position
 *
 * @details When a SFE order is encountered, there is no way to know which attributes will be set in the
 *          extended attribute pairs that follow the SFE, so this routine makea sure that any existing
 *          attribute settings are cleared before processing the pairs.
 */
void DisplayScreen::resetExtended(int pos)
{
    resetExtendedHilite(pos);

    cell.at(pos)->setColour(ColourTheme::BLUE);

    cell.at(pos)->setDisplay(true);
    cell.at(pos)->setNumeric(false);
    cell.at(pos)->setMDT(false);
    cell.at(pos)->setPenSelect(false);
    cell.at(pos)->setProtected(false);
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
    cell.at(pos)->setUnderscore(false);
    cell.at(pos)->setBlink(false);
    cell.at(pos)->setReverse(false);
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
    cell.at(pos)->setColour((ColourTheme::Colour)(c&7));
/*    if(foreground)
    {
        qDebug() << colName[cell.at(pos)->getColour()];
    }*/
}

/**
 * @brief   DisplayScreen::setExtendedBlink - switch blink on
 * @param   pos - screen position
 *
 * @details Set the cell at position to blink. Blink, Reverse and Uunderscore are mutually exclusive.
 */
void DisplayScreen::setExtendedBlink(int pos)
{
    // FIXME: Whast about underscore?
    cell.at(pos)->setReverse(false);
    cell.at(pos)->setBlink(true);
//    printf("[Blink]");
}

/**
 * @brief   DisplayScreen::setExtendedReverse - switch reverse on
 * @param   pos - screen position
 *
 * @details Set the cell at position to reverse. Blink, Reverse and Uunderscore are mutually exclusive.
 */
void DisplayScreen::setExtendedReverse(int pos)
{
    // FIXME: What about underscore?
    cell.at(pos)->setBlink(false);
    cell.at(pos)->setReverse(true);
//    printf("[Reverse]");
}

/**
 * @brief   DisplayScreen::setExtendedUscore - switch underscore on
 * @param   pos - screen position
 *
 * @details Set the cell at position to underscore. Blink, Reverse and Uunderscore are mutually exclusive.
 */
void DisplayScreen::setExtendedUscore(int pos)
{
    // FIXME: What about blink and reverse?
    cell.at(pos)->setUnderscore(true);
//    printf("[UScore]");
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
        if (cell.at(i)->isFieldStart() && cell.at(i)->isMdtOn())
        {
            cell.at(i)->setMDT(false);
        }

    }
}

/**
 * @brief   DisplayScreen::insertChar - Inserts or overwrites the character at the specified position
 * @param   pos        - position at which to insert character
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
    if (cell.at(cursor_pos)->isProtected() || cell.at(cursor_pos)->isFieldStart())
    {
        printf("Protected!\n");
        fflush(stdout);
        return false;
    }

    int thisField = cell.at(cursor_pos)->getField();

    if (insertMode)
    {
        /** TODO:
         *
         *  Insert only works when there is a null character in the field. If the field
         *  contains spaces, they don't count. The code below searches for the first null
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
        printf("This Field at: %d,%d, next field at %d,%d - last byte of this field %02X\n", (int)(thisField/screen_x), (int)(thisField-((int)(thisField/screen_x)*screen_x)), (int)(nextField/screen_x), (int)(nextField-((int)(nextField/screen_x)*screen_x)), cell.at(nextField - 1)->getEBCDIC() );
        uchar lastChar = cell.at(nextField - 1)->getEBCDIC();
        if (lastChar != IBM3270_CHAR_NULL && lastChar != IBM3270_CHAR_SPACE)
        {
            // Insert not okay
        }
        int endPos = -1;
        for(int i = cursor_pos; i < (cursor_pos + screenPos_max); i++)
        {
            int offset = i % screenPos_max;
            if (cell.at(offset)->isProtected() || cell.at(offset)->isFieldStart())
            {
                break;
            }
            if (cell.at(offset)->getEBCDIC() == IBM3270_CHAR_NULL)
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

            cell.at(offset)->copy(*(cell.at(offsetPrev)));
        }
    }

    cell.at(thisField)->setMDT(true);

    setChar(cursor_pos, c, true);

    setCursor((cursor_pos + 1) % screenPos_max);

    if (isAskip(cursor_pos))
    {
        tab(0);
    }

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
bool DisplayScreen::isAskip(int pos)
{
    return cell.at(pos)->isAutoSkip();
}

/**
 * @brief   DisplayScreen::isProtected - is this Cell protected?
 * @param   pos - screen position
 * @return  true if protected, false otherwise
 *
 * @details isProtected returns true if the Cell is protected.
 */
bool DisplayScreen::isProtected(int pos)
{
    return cell.at(pos)->isProtected();
}

/**
 * @brief   DisplayScreen::isFieldStart - is this Cell a Field Start
 * @param   pos - screen position
 * @return  true for a Field Start, false otherwise
 *
 * @details isFieldStart returns true if the Cell is a Field Start
 */
bool DisplayScreen::isFieldStart(int pos)
{
    return cell.at(pos)->isFieldStart();
}

/**
 * @brief   DisplayScreen::newline - move the cursor to the first input field after the current line
 *
 * @details Move the cursor to the first input field found after the start of the next line.
 */
void DisplayScreen::newline()
{
    int cursor_y = (cursor_pos / screen_x) + 1;

    if (cursor_y > screen_y)
    {
        cursor_y = 0;
    }

    cursor_pos = cursor_y * screen_x;

    tab(0);
}

/**
 * @brief   DisplayScreen::tab - tab to the next field
 * @param   offset - offset from the current position
 *
 * @details Move the cursor to the next input field, skipping the attribute byte.
 */
void DisplayScreen::tab(int offset)
{
    // Move cursor to next unprotected field, plus one to skip Field Start byte
    setCursor((findNextUnprotectedField(cursor_pos + offset) + 1) % screenPos_max);
}

/**
 * @brief   DisplayScreen::backtab - back to the previous field start
 *
 * @details Move the cursor to the start of the previous field (which may be this field)
 */
void DisplayScreen::backtab()
{
    setCursor((findPrevUnprotectedField(cursor_pos) + 1) % screenPos_max);
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
    if (cell.at(cursor_pos)->isProtected())
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

    for(int fld = cursor_pos; fld < endPos - 1 && cell.at(fld % screenPos_max)->getEBCDIC() != IBM3270_CHAR_NULL; fld++)
    {        
        int offset = fld % screenPos_max;
        int offsetNext = (fld + 1) % screenPos_max;

        cell.at(offset)->copy(*(cell.at(offsetNext)));
    }

    cell.at((endPos - 1) % screenPos_max)->setChar(IBM3270_CHAR_NULL);
    cell.at(cell.at(cursor_pos)->getField())->setMDT(true);
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
        cell.at(i % screenPos_max)->setChar(0x00);
    }

    cell.at(cell.at(cursor_pos)->getField())->setMDT(true);
}

/**
 * @brief   DisplayScreen::endline - move the cursor to the end of the current input field
 *
 * @details Move the cursor to the end of the text in the current input field.
 */
void DisplayScreen::endline()
{
    if (isProtected(cursor_pos))
    {
        return;
    }

    int endPos = cursor_pos + screenPos_max;

    int endField;

    int i = cursor_pos;
    int offset = cursor_pos;

    endField = cursor_pos;
    bool letter = false;

    while(i < endPos && !isProtected(offset) && !isFieldStart(offset))
    {
        uchar thisChar = cell.at(offset)->getChar().toLatin1();
        if (letter && (thisChar == 0x00 || thisChar == ' '))
        {
            endField = offset;
            letter = false;
        }

        if (thisChar != 0x00 && thisChar != ' ')
        {
            letter = true;
        }

        offset = ++i % screenPos_max;
    }

    setCursor(endField);
}

/**
 * @brief   DisplayScreen::home - move the cursor to the first field on the screen
 *
 * @details Move the cursor to the first field on the screen; searching starts at the very last position
 *          in case that is a field start, and the first position is not.
 */
void DisplayScreen::home()
{
    // Find first field on screen; this might be position 0, so we need to look starting at the last screen pos
    int nf = (findNextUnprotectedField(screenPos_max - 1) + 1) % screenPos_max;
    int cursor_y = (nf / screen_x);
    int cursor_x = nf - (cursor_y * screen_x);

    // Move cursor right (skip attribute byte)
    setCursor(cursor_x, cursor_y);
}

/**
 * @brief   DisplayScreen::backspace - backspace one character
 *
 * @details Backspace one character, stopping at the field start
 */
void DisplayScreen::backspace()
{
    // If we're at a protected field, do nothing
    if (isProtected(cursor_pos))
        return;

    // Discover whether the previous cursor position is a field start
    int tempCursorPos = cursor_pos == 0 ? screenPos_max - 1 : cursor_pos - 1;

    if (isFieldStart(tempCursorPos))
        return;

    // Backspace one character
    setCursor(tempCursorPos);
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

    int thisField = cell.at(start)->getField();

    if (cell.at(thisField)->isProtected())
    {
        start = findNextUnprotectedField(start);
    }

    for(int i = start; i < end; i++)
    {
        if(cell.at(i)->isProtected() || cell.at(i)->isFieldStart())
        {
            i = findNextUnprotectedField(i);
        }
        else
        {
                cell.at(i)->setChar(IBM3270_CHAR_SPACE);
        }
    }
}

/**
 * @brief   DisplayScreen::setCursorColour - set the cursor colour
 * @param   inherit - whether the cursor inherits the underlying character colour
 *
 * @details setCursorColour is called when the user changes the way the colour of the cursor is
 *          chosen. The default is for the cursor to be shown with the the colour of the Cell on
 *          which the cursor is placed, but it can be changed to be a static grey colour.
 */
void DisplayScreen::setCursorColour(bool inherit)
{
    cursorColour = inherit;
    if (inherit)
    {
        cursor.setBrush(palette[cell.at(cursor.data(0).toInt())->getColour()]);
    }
    else
    {
        cursor.setBrush(QBrush(QColor(0xBB, 0xBB, 0xBB)));
    }
    cursor.show();
}


/**
 * @brief   DisplayScreen::moveCursor - move the cursor
 * @param   x        - x position to move the cursor to
 * @param   y        - y position to move the cursor to

 */
void DisplayScreen::moveCursor(int x, int y)
{
    int tmpCursorPos = (cursor_pos + (y * screen_x + x)) % screenPos_max;

    if (tmpCursorPos < 0)
    {
        tmpCursorPos += screenPos_max;
    }

    setCursor(tmpCursorPos);
}

/**
 * @brief   DisplayScreen::setCursor - position cursor
 * @param   cursorPos - screen position
 *
 * @details setCursor is used when the cursor is moved either by the user or by the incoming 3270
 *          data stream.
 */
void DisplayScreen::setCursor(int cursorPos)
{
    int cursor_y = (cursorPos / screen_x);
    int cursor_x = cursorPos - (cursor_y * screen_x);

    setCursor(cursor_x, cursor_y);
}

/**
 * @brief   DisplayScreen::setCursor - position cursor
 * @param   x - screen position
 * @param   y - screen position
 *
 * @details setCursor is used when the cursor is moved either by the user or by the incoming 3270
 *          data stream.
 */
void DisplayScreen::setCursor(int x, int y)
{
    cursor.setVisible(false);

    cursor_pos = x + (y * screen_x);

    if (cursorColour)
    {
        if (cell.at(cursor_pos)->isReverse())
        {
            cursor.setBrush(palette[ColourTheme::BLACK]);
        }
        else
        {
            cursor.setBrush(palette[cell.at(cursor_pos)->getColour()]);
        }

    }

    cursor.setPos(gridSize_X * (qreal) x, gridSize_Y * (qreal) y);
//    cursor.setData(0,pos);

    cursor.setVisible(true);

    statusCursor.setText(QString("%1,%2").arg(x + 1, 3).arg(y + 1, -3));

    setRuler();
}

/**
 * @brief   DisplayScreen::showCursor - display cursor
 *
 * @details Called when the cursor blink is switched off to ensure that the cursor doesn't
 *          remain hidden if the blink happened to be at the point the cursor was hidden.
 */
void DisplayScreen::showCursor()
{
    cursor.show();
}

/**
 * @brief   DisplayScreen::setStatusXSystem - set XSystem text
 * @param   text - text to be shown
 *
 * @details Called when XSystem is to be shown or removed.
 */
void DisplayScreen::setStatusXSystem(QString text)
{
    statusXSystem.setText(text);
}

/**
 * @brief   DisplayScreen::setStatusInsert - the Insert mode text
 * @param   ins - true for insert mode, false for overwrite
 *
 * @details Called to show the Insert status on the status line.
 */
void DisplayScreen::setStatusInsert(bool ins)
{
    if (ins)
    {
        statusInsert.setText("\uFF3E");
    }
    else
    {
        statusInsert.setText("");
    }
}

/**
 * @brief   DisplayScreen::rulerMode - display/hide the ruler
 * @param   on - whether ruler is shown or not
 *
 * @details Called when Settings changes ruler to on or off.
 */
void DisplayScreen::rulerMode(bool on)
{

    rulerOn = on;
    setRuler();
}

/**
 * @brief  DisplayScreen::setRulerStyle - change ruler style
 * @param  rulerStyle - ruler style
 *
 *        rulerStyle | Description
 *        ---------- | -----------
 *          0        | Crosshair
 *          1        | Vertical
 *          2        | Horizontal
 *        other      | Off
 */
void DisplayScreen::setRulerStyle(int rulerStyle)
{
    this->ruler = rulerStyle;
    setRuler();
}

/**
 * @brief   DisplayScreen::toggleRuler - toggle the ruler on or off
 *
 * @details Called when the user switches the ruler on or off.
 */
void DisplayScreen::toggleRuler()
{
    // Invert ruler
    rulerOn = !rulerOn;

    setRuler();
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
 * @brief   DisplayScreen::setRuler - set the ruler style and redraw it in case it needs to move
 *
 * @details Called by several other routines when the ruler needs to be changed or the cursor has moved.
 */
void DisplayScreen::setRuler()
{
    if (rulerOn)
    {
        switch(ruler)
        {
            case Q3270_RULER_CROSSHAIR:
                crosshair_X.show();
                crosshair_Y.show();
                break;
            case Q3270_RULER_VERTICAL:
                crosshair_X.hide();
                crosshair_Y.show();
                break;
            case Q3270_RULER_HORIZONTAL:
                crosshair_X.show();
                crosshair_Y.hide();
        }
        int cursor_y = (cursor_pos / screen_x);
        int cursor_x = cursor_pos - (cursor_y * screen_x);
        crosshair_X.setPos((qreal) cursor_x * gridSize_X, 0);
        crosshair_Y.setPos(0 , (qreal) (cursor_y + 1) * gridSize_Y);
    }
    else
    {
        crosshair_X.hide();
        crosshair_Y.hide();
    }
}

/**
 * @brief   DisplayScreen::blink - blink the display
 *
 * @details Called by a timer in Terminal to blink any characters that have blink enabled
 */
void DisplayScreen::blink()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        if (cell.at(i)->isBlink())
        {
            // blinkChar is true for hiding the character
            cell.at(i)->blinkChar(!blinkShow);
        }
    }

    blinkShow = !blinkShow;
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
        if (cell.at(offset)->isFieldStart())
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
    if(cell.at(pos)->isFieldStart())
    {
        pos++;
    }
    int tmpPos;
    for(int i = pos; i < (pos + screenPos_max); i++)
    {
        tmpPos = i % screenPos_max;
        if (cell.at(tmpPos)->isFieldStart())
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
        if (cell.at(tmpPos)->isFieldStart() && !cell.at(tmpPos)->isProtected() && !cell.at(tmpNxt)->isFieldStart())
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
        if (cell.at(tmpPos)->isFieldStart() && !cell.at(tmpPos)->isProtected() && !cell.at(tmpNxt)->isFieldStart())
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
            if (cell.at(i)->isFieldStart() && !cell.at(i)->isProtected())
            {
                // This assumes that where two fields are adajcent to each other, the first cannot have MDT set
                if (cell.at(i)->isMdtOn() && !cell.at(i)->isProtected())
                {
                    buffer.append(IBM3270_SBA);

                    int nextPos = (i + 1) % screenPos_max;

                    addPosToBuffer(buffer, nextPos);

                    do
                    {
                        uchar b = cell.at(nextPos++)->getEBCDIC();
                        if (b != IBM3270_CHAR_NULL)
                        {
                            buffer.append(b);
                        }
                        nextPos = nextPos % screenPos_max;
                        //FIXME: Not sure this is right. This was a quick hack to cater for there being only one unprotected
                        //       field on the screen.
                        if (nextPos == 0)
                        {
                            printf("Wrapped!");
                            return;
                        }
                    }
                    while(!cell.at(nextPos)->isFieldStart());
                }
            }
        }
        else
        {
            uchar b = cell.at(i)->getEBCDIC();
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
    printf("Screen_X = %d, screen_Y =%d\n", screen_x, screen_y);
    fflush(stdout);
    for(int i = 0; i < screenPos_max; i++)
    {
        if (cell.at(i)->isFieldStart())
        {
            int tmpy = i / screen_x;
            int tmpx = i - (tmpy * screen_x);

            printf("Field at %4d (%2d,%2d) : Prot: %d\n", i, tmpx, tmpy, cell.at(i)->isProtected());
        }
    }
    fflush(stdout);
}

/**
 * @brief   DisplayScreen::dumpDisplay - print out a debug replication of the screen
 *
 * @details Used to debug screen layout
 */
void DisplayScreen::dumpDisplay()
{
    printf("---- SCREEN ----\n");

    QString ascii;

    for (int i = 0; i < screenPos_max; i++)
    {
        if (i % screen_x == 0 && i > 0)
        {

            printf("| %s |\n", ascii.toLatin1().data());
            ascii = "";
        }
        ascii.append(cell.at(i)->getChar());
        printf("%2.2X ", cell.at(i)->getEBCDIC());
    }

    printf("| %s |\n", ascii.toLatin1().data());

    printf("\n---- SCREEN ----\n");
    fflush(stdout);
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
    printf("\nCell at %d (%d, %d)\n", cursor_pos, x, y);
    printf("    Character: \"%c\" (hex %2.2X EBCDIC %2.2X)\n", cell.at(cursor_pos)->getChar().toLatin1(),cell.at(cursor_pos)->getChar().toLatin1(),cell.at(cursor_pos)->getEBCDIC());

    printf("    Field Attribute: %d\n", cell.at(cursor_pos)->isFieldStart());
    if (cell.at(cursor_pos)->isFieldStart())
    {
        printf("        MDT:       %d\n        Protected: %d\n        Numeric:   %d\n        Autoskip:  %d\n        Display:   %d\n",
                cell.at(cursor_pos)->isMdtOn(),
                cell.at(cursor_pos)->isProtected(),
                cell.at(cursor_pos)->isNumeric(),
                cell.at(cursor_pos)->isAutoSkip(),
                cell.at(cursor_pos)->isDisplay());
    }

    printf("    Extended: %d\n", cell.at(cursor_pos)->isExtended());
    if (cell.at(cursor_pos)->isExtended())
    {
        printf("        Intensify: %d\n        UScore:    %d\n        Reverse:   %d\n        Blink:     %d\n",
               cell.at(cursor_pos)->isIntensify(),
               cell.at(cursor_pos)->isUScore(),
               cell.at(cursor_pos)->isReverse(),
               cell.at(cursor_pos)->isBlink());
    }

    printf("    Character Attributes:\n        Extended: %d\n        CharSet:  %d\n        Colour:   %d\n    Colour: %d\n    Graphic: %d\n",
                cell.at(cursor_pos)->hasCharAttrs(Cell::EXTENDED),
                cell.at(cursor_pos)->hasCharAttrs(Cell::CHARSET),
                cell.at(cursor_pos)->hasCharAttrs(Cell::COLOUR),
                cell.at(cursor_pos)->getColour(),
                cell.at(cursor_pos)->isGraphic());

    fflush(stdout);

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
        if (cell.at(i)->isFieldStart())
        {
            buffer.append(IBM3270_SF);
            uchar attr;
            if (cell.at(i)->isDisplay() && !cell.at(i)->isPenSelect())
            {
                attr = 0x00;
            }
            else if (cell.at(i)->isDisplay() && cell.at(i)->isPenSelect())
            {
                attr = 0x01;
            }
            else if(cell.at(i)->isIntensify())
            {
                attr = 0x10;
            }
            else
            {
                attr = 0x11;
            }

            int byte = twelveBitBufferAddress[cell.at(i)->isMdtOn() | attr << 3 | cell.at(i)->isNumeric() << 4 | cell.at(i)->isProtected() << 5];

            buffer.append(byte);

            // Double up 0xFF bytes
            if (byte == 0xFF)
            {
                buffer.append(byte);
            }
        }
        else
        {
            buffer.append(cell.at(i)->getEBCDIC());
        }
    }
}

/**
 * @brief   DisplayScreen::mousePressEvent - process a mouse event
 * @param   mEvent - the event
 *
 * @details Called when a mouse event happens in DisplayScreen. This routine handles a left-click, and
 *          stores the coordinates of the click. The Rubberband is hidden (it will be shown if the mouse
 *          is moved).
 */
void DisplayScreen::mousePressEvent(QGraphicsSceneMouseEvent *mEvent)
{
    if (mEvent->button() != Qt::LeftButton)
    {
        mEvent->ignore();
        return;
    }

    int x = mEvent->pos().x() / gridSize_X;
    int y = mEvent->pos().y() / gridSize_Y;

    qDebug() << "Mouse press at" << mEvent->pos() << "- char pos" << x << "," << y << "scaled pos" << x * gridSize_X << "," << y * gridSize_Y;

    mouseStart = mapFromItem(this, QPoint(x * gridSize_X, y * gridSize_Y));

    myRb->setData(0, x);
    myRb->setData(1, y);
    myRb->setData(2, x);
    myRb->setData(3, y);

    myRb->hide();
}

/**
 * @brief   DisplayScreen::mouseMoveEvent - process a mouse move event
 * @param   mEvent - the event
 *
 * @details Called when the mouse is moved after a click. This routine calculates the Cells around which
 *          the rubberband is to be drawn and then makes it visible.
 */
void DisplayScreen::mouseMoveEvent(QGraphicsSceneMouseEvent *mEvent)
{   
    //FIXME: Some of this could probably be simplified so we're not working out
    //       the min/max values in mouseReleaseEvent

    // Calculate character position of mouse
    int thisX = mEvent->pos().x() / gridSize_X;
    int thisY = mEvent->pos().y() / gridSize_Y;

    // Normalise the position to within the bounds of the character display
    thisX = std::min(thisX, screen_x - 1);
    thisY = std::min(thisY, screen_y - 1);

    thisX = std::max(thisX, 0);
    thisY = std::max(thisY, 0);

    // Retrieve the start point of the selection
    int mpX = myRb->data(0).toInt();
    int mpY = myRb->data(1).toInt();

    /* Normalise the new mouse position
       If the user moves the mouse up and/or left, this is the new start point,
       and the old start point becomes the bottom right
    */
    int topLeftX = std::min(mpX, thisX);
    int topLeftY = std::min(mpY, thisY);

    int botRightX = std::max(thisX, mpX);
    int botRightY = std::max(thisY, mpY);

    myRb->setData(2, thisX);
    myRb->setData(3, thisY);

    // Add one to sizing to ensure rectangle moves when mouse moves to next character
    int w = botRightX - topLeftX + 1;
    int h = botRightY - topLeftY + 1;

    qDebug() << "Move" << mEvent->pos() << "this " << thisX << "," << thisY << " mpX,mpY" << mpX << "," << mpY << "    topLeftX,topLeftY" << topLeftX << "," << topLeftY << "    botRightX,botRightY" << botRightX << "," << botRightY << "w,h" << w << "x" << h;

    myRb->setRect(topLeftX * gridSize_X, topLeftY * gridSize_Y, w * gridSize_X, h * gridSize_Y);

    myRb->show();
}

/**
 * @brief   DisplayScreen::mouseReleaseEvent - process a mouse release event
 * @param   mEvent - the event
 *
 * @details Called when the left mouse button is released. If the mouse button was released without
 *          moving the mouse, the rubberband will be invisible, and this is interpreted as the user
 *          wishing to move the cursor by clicking somewhere in the display.
 */
void DisplayScreen::mouseReleaseEvent(QGraphicsSceneMouseEvent *mEvent)
{
    qDebug() << "Mouse release at " << mEvent->pos();

    // Single click, move cursor
    if (!myRb->isVisible())
    {
        qDebug() << "Single click";
        setCursor(myRb->data(0).toInt(), myRb->data(1).toInt());
        return;
    }

    int top = std::min(myRb->data(1).toInt(), myRb->data(3).toInt());
    int bottom = std::max(myRb->data(1).toInt(), myRb->data(3).toInt());

    int left = std::min(myRb->data(0).toInt(), myRb->data(2).toInt());
    int right = std::max(myRb->data(0).toInt(), myRb->data(2).toInt());

    qDebug() << "Selected" << left << "," << top << "x" << right << "," << bottom;
}

/**
 * @brief   DisplayScreen::copyText - copy the text within the rubberband to the clipboard
 *
 * @details Called when the user invokes the Copy function (default Ctrl-C) to copy the text
 *          contained within the rubberband region to the clipboard. Each new line within the
 *          rubberband generates a newline on the clipboard.
 */
void DisplayScreen::copyText()
{
    // If the rubberband isn't showing, do nothing
    if (!myRb->isVisible()) {
        return;
    }

    // Build up a string with the selected characters
    QString cbText = "";

    int top = std::min(myRb->data(1).toInt(), myRb->data(3).toInt());
    int bottom = std::max(myRb->data(1).toInt(), myRb->data(3).toInt());

    int left = std::min(myRb->data(0).toInt(), myRb->data(2).toInt());
    int right = std::max(myRb->data(0).toInt(), myRb->data(2).toInt());

    qDebug() << "Selection " << top << "," << left << " x " << bottom << "," << right;

    for(int y = top; y <= bottom; y++)
    {
        // Append a newline if there's more than one row selected
        if (y > top) {
            cbText = cbText + "\n";
        }

        for(int x = left; x <= right; x++)
        {
            int thisPos = screen_x * y + x;
            cbText = cbText + cell.at(thisPos)->getChar();
        }
    }

    qDebug() << "Clipboard text: " << cbText;

    QClipboard *clipboard = QGuiApplication::clipboard();

    clipboard->setText(cbText);

    myRb->hide();
}
