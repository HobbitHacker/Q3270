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

#include "Q3270.h"
#include "DisplayScreen.h"

/*
 * DisplayScreen represents a screen of the 3270 display. The class handles the display matrix.
 *
 * This is created by TerminalTab.
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
    blinkShow = false;
    cursorShow = true;
    cursorColour = true;

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

DisplayScreen::~DisplayScreen()
{
}

int DisplayScreen::width()
{
    return screen_x;
}

int DisplayScreen::height()
{
    return screen_y;
}

qreal DisplayScreen::gridWidth()
{
    return gridSize_X;
}

qreal DisplayScreen::gridHeight()
{
    return gridSize_Y;
}

void DisplayScreen::setFont(QFont font)
{
    for (int i = 0; i < screenPos_max; i++)
    {
        cell.at(i)->setFont(QFont(font));
    }
}

void DisplayScreen::setCodePage()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        cell.at(i)->refreshCodePage();
    }
}

void DisplayScreen::resetColours()
{
   for (int i = 0; i < screenPos_max; i++)
    {
        cell.at(i)->setColour(cell.at(i)->getColour());
    }
}

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
        cell.at(i)->setUScore(false);
        cell.at(i)->setReverse(false);
        cell.at(i)->setBlink(false);

        cell.at(i)->resetCharAttrs();

        cell.at(i)->setColour(ColourTheme::GREEN);

        cell.at(i)->setChar(0x00);

    }
    resetCharAttr();

    geActive = false;
    unformatted = true;
}

void DisplayScreen::setChar(int pos, short unsigned int c, bool move, bool fromKB)
{

    cell.at(pos)->setFieldStart(false);

    int fieldAttr = findField(pos);

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

    // If the character is not moving (as part of a delete/insert action) set character attributes if applicable
    // Character attributes move with the character otherwise.
    if (!move)
    {
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
                cell.at(pos)->setUScore(charAttr.uscore);
            }
            else
            {
                cell.at(pos)->setUScore(cell.at(fieldAttr)->isUScore());
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
            cell.at(pos)->setUScore(cell.at(fieldAttr)->isUScore());
            cell.at(pos)->setReverse(cell.at(fieldAttr)->isReverse());
            cell.at(pos)->setBlink(cell.at(fieldAttr)->isBlink());
        }
    }

}

unsigned char DisplayScreen::getChar(int pos)
{
    return (cell.at(pos)->getChar().toLatin1());
}

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

void DisplayScreen::resetCharAttr()
{
    charAttr.blink_default = true;
    charAttr.reverse_default = true;
    charAttr.uscore_default = true;
    charAttr.colour_default = true;

    useCharAttr = false;
}

void DisplayScreen::setGraphicEscape()
{
    geActive = true;
}

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

    // Field attributes do not have character attributes
    cell.at(pos)->resetCharAttrs();

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
        }
}

void DisplayScreen::refresh()
{
        for (int i = 0; i < screenPos_max; i++)
        {
            cell.at(i)->updateCell();
        }
}

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


void DisplayScreen::resetExtendedHilite(int pos)
{
    cell.at(pos)->setUScore(false);
    cell.at(pos)->setBlink(false);
    cell.at(pos)->setReverse(false);
}

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

void DisplayScreen::setExtendedBlink(int pos)
{
    cell.at(pos)->setReverse(false);
    cell.at(pos)->setBlink(true);
//    printf("[Blink]");
}

void DisplayScreen::setExtendedReverse(int pos)
{
    cell.at(pos)->setBlink(false);
    cell.at(pos)->setReverse(true);
//    printf("[Reverse]");
}

void DisplayScreen::setExtendedUscore(int pos)
{
    cell.at(pos)->setUScore(true);
//    printf("[UScore]");
}

/* Reset all MDTs in the display; it's probably faster to just loop through the entire buffer
 * rather than calling findNextField()
 *
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
 * @brief DisplayScreen::insertChar
 *        Inserts or overwrites a character at the specified position, resetting field attributes if
 *        required.
 * @param pos - position at which to insert character
 * @param c - character to be inserted
 * @param insertMode - true for insert, false for overtype
 * @return true if insert was successful, false if field protected or not enough space for insert mode
 */
bool DisplayScreen::insertChar(int pos, unsigned char c, bool insertMode)
{
    if (cell.at(pos)->isProtected() || cell.at(pos)->isFieldStart())
    {
        printf("Protected!\n");
        fflush(stdout);
        return false;
    }

    int thisField = findField(pos);

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
        int nextField = findNextField(pos);
        printf("This Field at: %d,%d, next field at %d,%d - last byte of this field %02X\n", (int)(thisField/screen_x), (int)(thisField-((int)(thisField/screen_x)*screen_x)), (int)(nextField/screen_x), (int)(nextField-((int)(nextField/screen_x)*screen_x)), cell.at(nextField - 1)->getEBCDIC() );
        uchar lastChar = cell.at(nextField - 1)->getEBCDIC();
        if (lastChar != IBM3270_CHAR_NULL && lastChar != IBM3270_CHAR_SPACE)
        {
            // Insert not okay
        }
        int endPos = -1;
        for(int i = pos; i < (pos + screenPos_max); i++)
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

        for(int fld = endPos; fld > pos; fld--)
        {
            int offset = fld % screenPos_max;
            int offsetPrev = (fld - 1) % screenPos_max;

            cell.at(offset)->copy(*(cell.at(offsetPrev)));
        }
    }

    cell.at(thisField)->setMDT(true);

    setChar(pos, c, false, true);

    return true;
}

/**
 * \class DisplayScreen::isAskip
 *
 * \brief isAskip returns a boolean indicating whether the supplied screen position contains askip.
 */
bool DisplayScreen::isAskip(int pos)
{
    return cell.at(pos)->isAutoSkip();
}

bool DisplayScreen::isProtected(int pos)
{
    return cell.at(pos)->isProtected();
}

bool DisplayScreen::isFieldStart(int pos)
{
    return cell.at(pos)->isFieldStart();
}

void DisplayScreen::deleteChar(int pos)
{
    if (cell.at(pos)->isProtected())
    {
        printf("Protected!\n");
        fflush(stdout);
        return;
    }

    int endPos = findNextField(pos);

    for(int fld = pos; fld < endPos - 1 && cell.at(fld % screenPos_max)->getEBCDIC() != IBM3270_CHAR_NULL; fld++)
    {        
        int offset = fld % screenPos_max;
        int offsetNext = (fld + 1) % screenPos_max;

        cell.at(offset)->copy(*(cell.at(offsetNext)));
    }

    cell.at(endPos - 1)->setChar(IBM3270_CHAR_NULL);
    cell.at(findField(pos))->setMDT(true);
}

void DisplayScreen::eraseEOF(int pos)
{
    int nextField = findNextField(pos);

    if (nextField < pos)
    {
        nextField += screenPos_max;
    }

    /* Blank field */
    for(int i = pos; i < nextField; i++)
    {
        cell.at(i % screenPos_max)->setChar(0x00);
    }

    cell.at(findField(pos))->setMDT(true);
}

void DisplayScreen::eraseUnprotected(int start, int end)
{
    if (end < start)
    {
        end += screenPos_max;
    }

    int thisField = findField(start);
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

void DisplayScreen::setCursor(int x, int y)
{
    cursor.setVisible(false);

    int pos = y * screen_x + x;

    if (cursorColour)
    {
        if (cell.at(pos)->isReverse())
        {
                cursor.setBrush(palette[ColourTheme::BLACK]);
        }
        else
        {
                cursor.setBrush(palette[cell.at(pos)->getColour()]);
        }

    }

    cursor.setPos(gridSize_X * (qreal) x, gridSize_Y * (qreal) y);
    cursor.setData(0,pos);

    cursor.setVisible(true);
}

/*!
 * \brief DisplayScreen::showCursor
 * \details Called when the cursor blink is switched off to ensure that the cursor doesn
 *  remain hidden if the blink happened to be at the point the cursor was hidden.
 */
void DisplayScreen::showCursor()
{
    cursor.show();
}

void DisplayScreen::setStatusXSystem(QString text)
{
    statusXSystem.setText(text);
}

void DisplayScreen::showStatusCursorPosition(int x, int y)
{
    statusCursor.setText(QString("%1,%2").arg(x + 1, 3).arg(y + 1, -3));
}

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

/*!
 * \brief DisplayScreen::rulerMode
 * \details Called when Settings changes ruler to on or off.
 * \param on - whether ruler is shown or not
 */
void DisplayScreen::rulerMode(bool on)
{

    rulerOn = on;
    setRuler();
}

void DisplayScreen::setRulerStyle(int rulerStyle)
{
    this->ruler = rulerStyle;
    setRuler();
}

void DisplayScreen::toggleRuler()
{
    // Invert ruler
    rulerOn = !rulerOn;

    setRuler();
}

void DisplayScreen::setRuler()
{
    //
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
    }
    else
    {
        crosshair_X.hide();
        crosshair_Y.hide();
    }
}

void DisplayScreen::drawRuler(int x, int y)
{
    if (rulerOn)
    {
       crosshair_X.setPos((qreal) x * gridSize_X, 0);
       crosshair_Y.setPos(0 , (qreal) (y + 1) * gridSize_Y);
    }
}

void DisplayScreen::blink()
{
    blinkShow = !blinkShow;

    for (int i = 0; i < screenPos_max; i++)
    {
        if (cell.at(i)->isBlink())
        {
            cell.at(i)->blinkChar(blinkShow);
        }
    }
}

void DisplayScreen::cursorBlink()
{
    cursorShow = !cursorShow;

    if (!cursorShow)
    {
        cursor.hide();
    }
    else
    {
        cursor.show();
    }
}

int DisplayScreen::findField(int pos)
{
    int endPos = pos - screenPos_max;

    for (int i = pos; i > endPos ; i--)
    {
        int offset = i;
        if (i < 0)
        {
            offset = screenPos_max + i;
        }

        if (cell.at(offset)->isFieldStart())
        {
            return offset;
        }
    }
    return pos;
}

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

int DisplayScreen::findNextUnprotectedField(int pos)
{
 /*----------------------------------------------------------------------------------
  | Find the next field that is unprotected. This incorporates two field start      |
  | attributes next to each other - field start attributes are protected.           |
  ----------------------------------------------------------------------------------*/
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

int DisplayScreen::findPrevUnprotectedField(int pos)
{
 /*----------------------------------------------------------------------------------
  | Find the previous field that is unprotected. This incorporates two field start  |
  | attributes next to each other - field start attributes are protected.           |
  ----------------------------------------------------------------------------------*/
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
 *  @brief DisplayScreen::getModifiedFields
 *         Utility method to extract all modified fields from the screen and add them to the provided buffer
 *  @param buffer - address of a QByteArray to which the modified fields are appended
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
 * \brief DisplayScreen::addPosToBuffer
 *        Utility method to insert 'pos' into 'buffer' as two bytes, doubling 0xFF if needed.
 * \param buffer
 * \param pos
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

//INFO: Not used, but for debugging
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

void DisplayScreen::dumpInfo(int pos)
{
    int y = pos / screen_x;
    int x = pos - y * screen_x;
    printf("\nCell at %d (%d, %d)\n", pos, x, y);
    printf("    Character: \"%c\" (hex %2.2X EBCDIC %2.2X)\n", cell.at(pos)->getChar().toLatin1(),cell.at(pos)->getChar().toLatin1(),cell.at(pos)->getEBCDIC());

    printf("    Field Attribute: %d\n", cell.at(pos)->isFieldStart());
    if (cell.at(pos)->isFieldStart())
    {
        printf("        MDT:       %d\n        Protected: %d\n        Numeric:   %d\n        Autoskip:  %d\n        Display:   %d\n",
                cell.at(pos)->isMdtOn(),
                cell.at(pos)->isProtected(),
                cell.at(pos)->isNumeric(),
                cell.at(pos)->isAutoSkip(),
                cell.at(pos)->isDisplay());
    }

    printf("    Extended: %d\n", cell.at(pos)->isExtended());
    if (cell.at(pos)->isExtended())
    {
        printf("        Intensify: %d\n        UScore:    %d\n        Reverse:   %d\n        Blink:     %d\n",
               cell.at(pos)->isIntensify(),
               cell.at(pos)->isUScore(),
               cell.at(pos)->isReverse(),
               cell.at(pos)->isBlink());
    }

    printf("    Character Attributes:\n        Extended: %d\n        CharSet:  %d\n        Colour:   %d\n    Colour: %d\n    Graphic: %d\n",
                cell.at(pos)->hasCharAttrs(Cell::EXTENDED),
                cell.at(pos)->hasCharAttrs(Cell::CHARSET),
                cell.at(pos)->hasCharAttrs(Cell::COLOUR),
                cell.at(pos)->getColour(),
                cell.at(pos)->isGraphic());

    fflush(stdout);

}

void DisplayScreen::getScreen(QByteArray &buffer)
{
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

void DisplayScreen::mouseReleaseEvent(QGraphicsSceneMouseEvent *mEvent)
{
    qDebug() << "Mouse release at " << mEvent->pos();

    // Single click, move cursor
    if (!myRb->isVisible())
    {
        qDebug() << "Single click";
        emit moveCursor(myRb->data(0).toInt(), myRb->data(1).toInt(), true);
        return;
    }

    int top = std::min(myRb->data(1).toInt(), myRb->data(3).toInt());
    int bottom = std::max(myRb->data(1).toInt(), myRb->data(3).toInt());

    int left = std::min(myRb->data(0).toInt(), myRb->data(2).toInt());
    int right = std::max(myRb->data(0).toInt(), myRb->data(2).toInt());

    qDebug() << "Selected" << left << "," << top << "x" << right << "," << bottom;
}

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
