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

#include <arpa/telnet.h>

#include "Q3270.h"
#include "DisplayScreen.h"

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
DisplayScreen::DisplayScreen(int screen_x, int screen_y, CodePage &cp, const Colours *palette, QGraphicsScene *scene) : cp(cp), palette(palette), screen_x(screen_x), screen_y(screen_y)
{

    // 3270 screens are (were) 4:3 ratio, so use a reasonable size that Qt can scale.
    this->setRect(0, 0, 640, 480);
    this->setPos(0, 0);

    gridSize_X = (qreal) 640 / (qreal) screen_x;
    gridSize_Y = (qreal) 480 / (qreal) screen_y;

    screenPos_max = screen_x * screen_y;

    // Default settings
    ruler = Q3270::CrossHair;
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

            cell.replace(pos, new Cell(pos, x_pos, y_pos, gridSize_X, gridSize_Y, cp, palette, this, scene));
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

    QFontMetrics fm = QFontMetrics(statusBarText);

    // Connect status at 0% across
    statusConnect.setText("4-A");
    statusConnect.setPos(0, 481);
    statusConnect.setFont(statusBarText);
    statusConnect.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    unlock = new ClickableSvgItem(":/Icons/unlock.svg");
    unlock->setToolTip("Unsecured Connection");

    lock = new ClickableSvgItem(":/Icons/lock.svg");
    lock->setToolTip("Secured Connection, but certificate chain is not secure");

    locktick = new ClickableSvgItem(":/Icons/lock-tick.svg");
    locktick->setToolTip("Secured Connection");

    // Connect status at 5% across
    statusSecure.setPos(gridSize_X * (screen_x * .05), 482);
    statusSecure.setRect(0, 0, 9, 9);

    QRectF r = lock->boundingRect();

    lock->setScale(8 / r.height());
    unlock->setScale(8 / r.height());
    locktick->setScale(8 / r.height());

    unlock->setParentItem(&statusSecure);
    lock->setParentItem(&statusSecure);
    locktick->setParentItem(&statusSecure);

    lock->hide();
    locktick->hide();

    // XSystem 20% across status bar
    statusX = new LockIndicator();
    statusX->setPos(gridSize_X * (screen_x * .20), 481);

//    statusXSystem.setText("X System");
//    statusXSystem.setPos(gridSize_X * (screen_x * .20), 481);
//    statusXSystem.setFont(statusBarText);
//    statusXSystem.setBrush(QBrush(QColor(0xFF, 0xFF, 0xFF)));

    // X clock 20% across status bar
//    QGraphicsSimpleTextItem  *statusXClockText = new QGraphicsSimpleTextItem("X");
//    statusXClockText->setPos(gridSize_X * (screen_x * .35), 481);
//    statusXClockText->setFont(statusBarText);
//    statusXClockText->setBrush(QBrush(QColor(0xFF, 0xFF, 0xFF)));

//    statusXClockIcon = new QGraphicsSvgItem(":/Icons/clock.svg");
    //    auto effect = new QGraphicsColorizeEffect;
    //    effect->setColor(Qt::white);
    //    statusXClockIcon->setGraphicsEffect(effect);

    //    r = statusXClockIcon->boundingRect();
    //    statusXClockIcon->setScale(7 / r.height());

//    statusXClockIcon->setPos(gridSize_X * (screen_x * .37), 483);

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
    statusCursor.setToolTip("Cursor position (y,x)");

    scene->addItem(&statusBar);
    scene->addItem(&statusSecure);
    scene->addItem(&statusConnect);
//    scene->addItem(&statusXSystem);
    scene->addItem(statusX);
    scene->addItem(&statusCursor);
    scene->addItem(&statusInsert);

//    statusXClock = scene->createItemGroup({statusXClockText, statusXClockIcon});
//    statusXClock->hide();

//    statusXSystem.hide();

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
qreal DisplayScreen::gridWidth() const
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
qreal DisplayScreen::gridHeight() const
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

        cell.at(i)->setColour(Q3270::Green);

        cell.at(i)->setChar(0x00);
        cell.at(i)->setField(nullptr);
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

    // If we're overlaying a Field Start, the subsequent positions on the screen now have a different
    // field start, so update them accordingly.
    if (cell.at(pos)->isFieldStart())
    {
        cell.at(pos)->setFieldStart(false);

        int lastField;

        // If this is the first position on the screen, we need the last screen position's field
        lastField = cell.at(pos == 0 ? screenPos_max - 1 : pos - 1)->getField();

        int tmpPos = pos;

        while(!cell.at(tmpPos % screenPos_max)->isFieldStart() && tmpPos < pos + screenPos_max)
        {
            int i1 = tmpPos++ % screenPos_max;
//          qDebug() << "Field at" << pos << "was FieldStart. Updating" << i1 << "as" << lastField;
            cell.at(i1)->setField(cell.at(lastField));
        }
    }

    int fieldAttr = cell.at(pos)->getField();

    // If the field attribute is not set, use the current position
    if (fieldAttr == -1)
        fieldAttr = pos;

    // Set character attribute flags if applicable
    if (useCharAttr)
        applyCharAttributes(pos, fieldAttr);

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

    // If character colour attributes are present, use them instead
    if (cell.at(pos)->hasCharAttrs(Q3270::ColourAttr) || cell.at(pos)->hasCharAttrs(Q3270::ExtendedAttr))
        applyCharAttrsOverrides(pos, fieldAttr);
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
        while(i < endPos && !(cell.at(i % screenPos_max)->isFieldStart()))
        {
            int offset = i++ % screenPos_max;
            cell.at(offset)->setField(cell.at(pos));
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
        resetCharAttr();
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

    cell.at(pos)->setColour(Q3270::Blue);

    cell.at(pos)->setDisplay(true);
    cell.at(pos)->setNumeric(false);
    cell.at(pos)->setMDT(false);
    cell.at(pos)->setPenSelect(false);
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
    cell.at(pos)->setColour((Q3270::Colour)(c&7));
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
    cell.at(pos)->setUnderscore(false);
    cell.at(pos)->setReverse(false);
    cell.at(pos)->setBlink(true);
}

/**
 * @brief   DisplayScreen::setExtendedReverse - switch reverse on
 * @param   pos - screen position
 *
 * @details Set the cell at position to reverse. Blink, Reverse and Underscore are mutually exclusive.
 */
void DisplayScreen::setExtendedReverse(int pos)
{
    cell.at(pos)->setUnderscore(false);
    cell.at(pos)->setBlink(false);
    cell.at(pos)->setReverse(true);
}

/**
 * @brief   DisplayScreen::setExtendedUscore - switch underscore on
 * @param   pos - screen position
 *
 * @details Set the cell at position to underscore. Blink, Reverse and Underscore are mutually exclusive.
 */
void DisplayScreen::setExtendedUscore(int pos)
{
    cell.at(pos)->setBlink(false);
    cell.at(pos)->setReverse(false);
    cell.at(pos)->setUnderscore(true);
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
  /*        qDebug() << "Resetting MDT at" << i;*/
            cell.at(i)->setMDT(false);
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
    if (cell.at(cursor_pos)->isProtected() || cell.at(cursor_pos)->isFieldStart())
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

    cell.at(cursor_pos)->setMDT(true);

    setChar(cursor_pos, c, true);

    cell.at(cursor_pos)->updateCell();

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
bool DisplayScreen::isAskip(int pos) const
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
bool DisplayScreen::isProtected(int pos) const
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
bool DisplayScreen::isFieldStart(int pos) const
{
    return cell.at(pos)->isFieldStart();
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
    cell.at(cursor_pos)->setMDT(true);
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

    cell.at(cursor_pos)->setMDT(true);
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

    if (cell.at(start)->isProtected())
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
    int tmpPos;

    if(cell.at(pos)->isFieldStart())
    {
        pos++;
    }

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
            if (!cell.at(i)->isProtected())
            {
                if (cell.at(i)->isFieldStart())
                {
                    qDebug() << "Input field found at " << i << "MDT is" << cell.at(i)->isMdtOn();
                    // This assumes that where two fields are adajcent to each other, the first cannot have MDT set
                    if (cell.at(i)->isMdtOn())
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

            if (cell.at(tmppos)->isFieldStart())
                if (cell.at(tmppos)->isMdtOn())
                    line.append("F");
                else
                    line.append("f");
            else if (cell.at(cell.at(tmppos)->getField())->isFieldStart())
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
    qDebug() << "---- SCREEN ----";

    QString ascii;
    QString hexline;

    for (int i = 0; i < screenPos_max; i++)
    {
        if (i % screen_x == 0 && i > 0)
        {

            qDebug() << hexline << "|" << ascii.toLatin1().data() << "|";
            hexline = "";
            ascii = "";
        }
        ascii.append(cell.at(i)->getChar());
        hexline.append(QString::asprintf("%02X ", cell.at(i)->getEBCDIC()));
    }

    if (!hexline.isEmpty())
        qDebug() << hexline << "|" << ascii.toLatin1().data() << "|";

    qDebug() << "---- SCREEN ----";
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

    qDebug() << QString::asprintf("Cell at %d (%d, %d)", cursor_pos, x, y);
    qDebug() << QString::asprintf("    Character: \"%c\" (hex %2.2X EBCDIC %2.2X)", cell.at(cursor_pos)->getChar().toLatin1(),cell.at(cursor_pos)->getChar().toLatin1(),cell.at(cursor_pos)->getEBCDIC());

    qDebug() << QString::asprintf("    Field Attribute: %d", cell.at(cursor_pos)->isFieldStart());
    qDebug() << QString::asprintf("        MDT:       %d\n        Protected: %d\n        Numeric:   %d\n        Autoskip:  %d\n        Display:   %d",
            cell.at(cursor_pos)->isMdtOn(),
            cell.at(cursor_pos)->isProtected(),
            cell.at(cursor_pos)->isNumeric(),
            cell.at(cursor_pos)->isAutoSkip(),
            cell.at(cursor_pos)->isDisplay());

    qDebug() << QString::asprintf("    Extended: %d\n", cell.at(cursor_pos)->isExtended());
    printf("        Intensify: %d\n        UScore:    %d\n        Reverse:   %d\n        Blink:     %d",
           cell.at(cursor_pos)->isIntensify(),
           cell.at(cursor_pos)->isUScore(),
           cell.at(cursor_pos)->isReverse(),
           cell.at(cursor_pos)->isBlink());

    qDebug() << QString::asprintf("    Character Attributes:\n        Extended: %d\n        CharSet:  %d\n        Colour:   %d\n    Colour: %d\n    Graphic: %d",
                cell.at(cursor_pos)->hasCharAttrs(Q3270::ExtendedAttr),
                cell.at(cursor_pos)->hasCharAttrs(Q3270::CharsetAttr),
                cell.at(cursor_pos)->hasCharAttrs(Q3270::ColourAttr),
                cell.at(cursor_pos)->getColour(),
                cell.at(cursor_pos)->isGraphic());

    int fieldStart = cell.at(cursor_pos)->getField();
    qDebug() << QString::asprintf("    Field Position: %d (%d, %d)", fieldStart, (int) (fieldStart / screen_x), (int) (fieldStart - (int) (fieldStart / screen_x) * screen_x));

    dumpFields();
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
 * @brief   DisplayScreen::applyCharAttributes - apply the character attributes to the cell
 * @param   pos       - screen position
 * @param   fieldAttr - field attribute
 *
 * @details Apply the character attributes to the cell at pos. This is used when the datastream
 *          selected a different colour for the specified cell.
 */
void DisplayScreen::applyCharAttributes(int pos, int fieldAttr)
{
    if (!charAttr.colour_default)
        cell.at(pos)->setCharAttrs(Q3270::ColourAttr, true);
    else
        cell.at(pos)->setColour(cell.at(fieldAttr)->getColour());

    if (!charAttr.blink_default)
        cell.at(pos)->setCharAttrs(Q3270::ExtendedAttr, true);
    else
        cell.at(pos)->setBlink(cell.at(fieldAttr)->isBlink());

    if (!charAttr.reverse_default)
        cell.at(pos)->setCharAttrs(Q3270::ExtendedAttr, true);
    else
        cell.at(pos)->setReverse(cell.at(fieldAttr)->isReverse());

    if (!charAttr.uscore_default)
        cell.at(pos)->setCharAttrs(Q3270::ExtendedAttr, true);
    else
        cell.at(pos)->setUnderscore(cell.at(fieldAttr)->isUScore());
}

/**
 * @brief   DisplayScreen::applyCharAttrsOverrides - apply the character attributes to the cell
 * @param   pos       - screen position
 * @param   fieldAttr - field attribute
 *
 * @details Apply the character attributes to the cell at pos. This is used when the datastream
 *          selected a different colour for the specified cell.
 */
void DisplayScreen::applyCharAttrsOverrides(int pos, int fieldAttr)
{
    // Colour
    if (!charAttr.colour_default)
        cell.at(pos)->setColour(charAttr.colNum);
    else
        cell.at(pos)->setColour(cell.at(fieldAttr)->getColour());

    // Reverse
    if (!charAttr.reverse_default)
        cell.at(pos)->setReverse(charAttr.reverse);
    else
        cell.at(pos)->setReverse(cell.at(fieldAttr)->isReverse());

    // Underscore
    if (!charAttr.uscore_default)
        cell.at(pos)->setUnderscore(charAttr.uscore);
    else
        cell.at(pos)->setUnderscore(cell.at(fieldAttr)->isUScore());

    // Blink
    if (!charAttr.blink_default)
        cell.at(pos)->setBlink(charAttr.blink);
    else
        cell.at(pos)->setBlink(cell.at(fieldAttr)->isBlink());
}
