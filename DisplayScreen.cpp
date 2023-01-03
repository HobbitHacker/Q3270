#include "Q3270.h"

#include "DisplayScreen.h"

DisplayScreen::DisplayScreen(int screen_x, int screen_y, ColourTheme::Colours colours, CodePage *cp)
{

    palette = colours;

    setBackgroundBrush(palette[ColourTheme::BLACK]);

    this->screen_x = screen_x;
    this->screen_y = screen_y;

    gridSize_X = 640*5 / screen_x;
    gridSize_Y = 480*5 / screen_y;

    screenPos_max = screen_x * screen_y;

    // Default settings
    fontScaling = true;
    ruler = Q3270_RULER_CROSSHAIR;
    rulerOn = false;
    blinkShow = false;
    cursorShow = true;
    cursorColour = true;

    // Build 3270 display matrix
    glyph.resize(screenPos_max);
    uscore.resize(screenPos_max);
    cell.resize(screenPos_max);

    for(int y = 0; y < screen_y; y++)
    {
        qreal y_pos = y * gridSize_Y;

        for(int x = 0; x < screen_x; x++)
        {
            int pos = x + (y * screen_x);

            qreal x_pos = x * gridSize_X;

            cell.replace(pos, new QGraphicsRectItem(0, 0, gridSize_X, gridSize_Y));
            cell.at(pos)->setBrush(QBrush(Qt::red));
            cell.at(pos)->setPen(Qt::NoPen);
            cell.at(pos)->setZValue(0);

            glyph.replace(pos, new Glyph(x, y, cp, NULL));
            glyph.at(pos)->setFlag(QGraphicsItem::ItemIsSelectable);
            glyph.at(pos)->setZValue(2);

            uscore.replace(pos, new QGraphicsLineItem(0, 0, gridSize_X, 0));
            uscore.at(pos)->setZValue(1);

            addItem(glyph.at(pos));
            addItem(uscore.at(pos));
            addItem(cell.at(pos));

            cell.at(pos)->setPos(x_pos, y_pos);
            glyph.at(pos)->setPos(x_pos, y_pos);
            uscore.at(pos)->setPos(x_pos, y_pos + gridSize_Y);
        }
    }

    // Set default attributes for initial power-on
    clear();
    setFont(QFont("ibm3270", 11));

    // Set up cursor
    cursor.setRect(cell.at(0)->rect());
    cursor.setPos(cell.at(0)->boundingRect().left(), cell.at(0)->boundingRect().top());
    cursor.setBrush(Qt::lightGray);
    cursor.setOpacity(0.5);
    cursor.setPen(Qt::NoPen);
    cursor.setParentItem(cell.at(0));

    // Set up crosshairs
    crosshair_X.setLine(0, 0, 0, screen_y * gridSize_Y);
    crosshair_Y.setLine(0, 0, screen_x * gridSize_X, 0);

    crosshair_X.setPen(QPen(Qt::white, 0));
    crosshair_Y.setPen(QPen(Qt::white, 0));

    crosshair_X.setZValue(2);
    crosshair_Y.setZValue(2);

    addItem(&crosshair_X);
    addItem(&crosshair_Y);

    crosshair_X.hide();
    crosshair_Y.hide();

    // Build status bar
    int statusPos = (screen_y * gridSize_Y + 2);

    statusBar.setLine(0, 0, screen_x * gridSize_X, 0);
    statusBar.setPos(0, statusPos++);
    statusBar.setPen(QPen(QColor(0x80, 0x80, 0xFF), 0));
    addItem(&statusBar);

    QFont statusBarText = QFont("ibm3270");
    statusBarText.setPixelSize(gridSize_Y * .75);

    // Connect status at 0% across
    statusConnect.setText("4-A");
    statusConnect.setPos(0, statusPos);
    statusConnect.setFont(statusBarText);
    statusConnect.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    // XSystem 20% across status bar
    statusXSystem.setText("");
    statusXSystem.setPos(gridSize_X * (screen_x * .20), statusPos);
    statusXSystem.setFont(statusBarText);
    statusXSystem.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    // Insert 50% across status bar
    statusInsert.setText("");
    statusInsert.setPos(gridSize_X * (screen_x * .50), statusPos);
    statusInsert.setFont(statusBarText);
    statusInsert.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    // Cursor 75% across status bar
    statusCursor.setText("");
    statusCursor.setPos(gridSize_X * (screen_x * .75), statusPos);
    statusCursor.setFont(statusBarText);
    statusCursor.setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    addItem(&statusConnect);
    addItem(&statusXSystem);
    addItem(&statusCursor);
    addItem(&statusInsert);
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
    termFont = font;
    QTransform tr;

    font.setKerning(false);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0);

    if (fontScaling)
    {
        QFontMetricsF fm = QFontMetrics(font);
        QRectF boxRect = QRectF(0, 0, fm.horizontalAdvance("┼", 1), fm.height());
        tr.scale(gridSize_X / boxRect.width(), gridSize_Y / boxRect.height());
    }
    else
    {
        tr.scale(1,1);
    }

    for (int i = 0; i < screenPos_max; i++)
    {
        glyph.at(i)->setFont(QFont(font));
        glyph.at(i)->setTransform(tr);
    }
}

void DisplayScreen::setCodePage()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        glyph.at(i)->refreshCodePage();
    }
}

void DisplayScreen::setColourPalette(ColourTheme::Colours c)
{
    palette = c;
}

void DisplayScreen::resetColours()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        if (glyph.at(i)->isReverse())
        {
            cell.at(i)->setBrush(palette[glyph.at(i)->getColour()]);
            glyph.at(i)->setBrush(palette[ColourTheme::BLACK]);
            printf("<reverse>");
        }
        else
        {
            glyph.at(i)->setBrush(palette[glyph.at(i)->getColour()]);
            cell.at(i)->setBrush(palette[ColourTheme::BLACK]);
        }
        if (!glyph.at(i)->isDisplay())
        {
            glyph.at(i)->setBrush(cell.at(i)->brush());
        }
    }
}

void DisplayScreen::setFontScaling(bool fontScaling)
{
    if (this->fontScaling != fontScaling)
    {
        this->fontScaling = fontScaling;
        setFont(termFont);
    }
}

void DisplayScreen::clear()
{
    for(int i = 0; i < screenPos_max; i++)
    {
        uscore.at(i)->setVisible(false);

        glyph.at(i)->setFieldStart(false);

        glyph.at(i)->setNumeric(false);
        glyph.at(i)->setMDT(false);
        glyph.at(i)->setProtected(false);
        glyph.at(i)->setDisplay(true);
        glyph.at(i)->setPenSelect(false);
        glyph.at(i)->setIntensify(false);

        glyph.at(i)->setExtended(false);
        glyph.at(i)->setUScore(false);
        glyph.at(i)->setReverse(false);
        glyph.at(i)->setBlink(false);

        glyph.at(i)->resetCharAttrs();

        cell.at(i)->setBrush(palette[ColourTheme::BLACK]);

        glyph.at(i)->setColour(ColourTheme::BLUE);
        glyph.at(i)->setBrush(palette[ColourTheme::BLUE]);
        glyph.at(i)->setText(0x00);

    }
    resetCharAttr();

    geActive = false;
}

void DisplayScreen::setChar(int pos, short unsigned int c, bool move, bool fromKB)
{

    glyph.at(pos)->setFieldStart(false);

    int lastField = findField(pos);

    glyph.at(pos)->resetCharAttrs();

    // Set character attribute flags if applicable
    if (!fromKB && useCharAttr)
    {
        if (!charAttr.colour_default)
        {
            glyph.at(pos)->setCharAttrs(true, Glyph::CharAttr::COLOUR);
        }
        if (!charAttr.blink_default)
        {
            glyph.at(pos)->setCharAttrs(true, Glyph::CharAttr::EXTENDED);
        }
        if (!charAttr.reverse_default)
        {
            glyph.at(pos)->setCharAttrs(true, Glyph::CharAttr::EXTENDED);
        }
        if (!charAttr.uscore_default)
        {
            glyph.at(pos)->setCharAttrs(true, Glyph::CharAttr::EXTENDED);
        }
    }

    // Non-display comes from field attribute
    glyph.at(pos)->setDisplay(glyph.at(lastField)->isDisplay());

    // Protected comes from the field attribute
    glyph.at(pos)->setProtected(glyph.at(lastField)->isProtected());

    // Choose a graphic character if needed
    glyph.at(pos)->setGraphic(geActive);

    if (!fromKB)
    {
        glyph.at(pos)->setText(c);
    }
    else
    {
        glyph.at(pos)->setTextFromKB(c);
    }

    geActive = false;

    // If the character is not moving (delete/insert action) set character attributes if applicable
    if (!move)
    {
        // If character colour attributes are present, use them
        if (glyph.at(pos)->hasCharAttrs(Glyph::CharAttr::COLOUR))
        {
            // Colour
            if (!charAttr.colour_default)
            {
                glyph.at(pos)->setColour(charAttr.colNum);
            }
            else
            {
                glyph.at(pos)->setColour(glyph.at(lastField)->getColour());
            }
        }
        else
        {
            glyph.at(pos)->setColour(glyph.at(lastField)->getColour());
        }

        if (glyph.at(pos)->hasCharAttrs(Glyph::CharAttr::EXTENDED))
        {
            // Reverse video
            if (!charAttr.reverse_default)
            {
                glyph.at(pos)->setReverse(charAttr.reverse);
            }
            else
            {
                glyph.at(pos)->setReverse(glyph.at(lastField)->isReverse());
            }

            // Underscore
            if (!charAttr.uscore_default)
            {
                glyph.at(pos)->setUScore(charAttr.uscore);
            }
            else
            {
                glyph.at(pos)->setUScore(glyph.at(lastField)->isUScore());
            }

            // Blink
            if (!charAttr.blink_default)
            {
                glyph.at(pos)->setBlink(charAttr.blink);
            }
            else
            {
                glyph.at(pos)->setBlink(glyph.at(lastField)->isBlink());
            }
        }
        else
        {
            glyph.at(pos)->setUScore(glyph.at(lastField)->isUScore());
            glyph.at(pos)->setReverse(glyph.at(lastField)->isReverse());
            glyph.at(pos)->setBlink(glyph.at(lastField)->isBlink());
        }
    }

    // Colour - non-display / reverse / normal
    if (!glyph.at(pos)->isDisplay())
    {
        glyph.at(pos)->setBrush(cell.at(pos)->brush());
    }
    else
    {
        if (glyph.at(pos)->isReverse())
        {
            cell.at(pos)->setBrush(palette[glyph.at(pos)->getColour()]);
            glyph.at(pos)->setBrush(palette[ColourTheme::BLACK]);
        }
        else
        {
            glyph.at(pos)->setBrush(palette[glyph.at(pos)->getColour()]);
            cell.at(pos)->setBrush(palette[ColourTheme::BLACK]);
        }
    }

    //
    // Underscore processing
    //

    if (glyph.at(pos)->isUScore())
    {
        uscore.at(pos)->setVisible(true);
        uscore.at(pos)->setPen(QPen(palette[glyph.at(pos)->getColour()],0));
    }
    else
    {
        uscore[pos]->setVisible(false);
    }
    printf("%s", glyph.at(pos)->text().toLatin1().data());
}

unsigned char DisplayScreen::getChar(int pos)
{
    return (glyph.at(pos)->text().toUtf8()[0]);
}


void DisplayScreen::setCharAttr(unsigned char extendedType, unsigned char extendedValue)
{
    printf("[SetAttribute ");

    switch(extendedType)
    {
        case IBM3270_EXT_DEFAULT:
            charAttr.blink_default = true;
            charAttr.reverse_default = true;
            charAttr.uscore_default = true;
            charAttr.colour_default = true;
            printf("default");
            break;
        case IBM3270_EXT_HILITE:
            switch(extendedValue)
            {
                case IBM3270_EXT_HI_DEFAULT:
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink   = false;
                    printf("default");
                    break;
                case IBM3270_EXT_HI_NORMAL:
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink   = false;
                    printf("normal");
                    break;
                case IBM3270_EXT_HI_BLINK:
                    charAttr.blink   = true;
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink_default = false;
                    printf("blink");
                    break;
                case IBM3270_EXT_HI_REVERSE:
                    charAttr.blink   = false;
                    charAttr.uscore  = false;
                    charAttr.reverse = true;
                    charAttr.reverse_default = false;
                    printf("reverse");
                    break;
                case IBM3270_EXT_HI_USCORE:
                    charAttr.blink   = false;
                    charAttr.reverse = false;
                    charAttr.uscore  = true;
                    charAttr.uscore_default = false;
                    printf("uscore");
                    break;
                default:
                    printf("** Extended Value %02X Not Implemented **", extendedValue);
            }
            break;
        case IBM3270_EXT_FG_COLOUR:
            if (extendedValue == IBM3270_EXT_DEFAULT)
            {
                charAttr.colour_default = true;
                printf("fg colour default");
            }
            else
            {
                charAttr.colour = palette[(ColourTheme::Colour)(extendedValue&7)];
                charAttr.colNum = (ColourTheme::Colour)(extendedValue&7);
                charAttr.colour_default = false;
                printf("fg colour %s (extendedValue %02X)", colName[charAttr.colNum], extendedValue);
            }
            break;
        case IBM3270_EXT_BG_COLOUR:
            if (extendedValue == IBM3270_EXT_DEFAULT)
            {
                charAttr.colour_default = true;
                printf("bg colour default");
            }
            else
            {
                charAttr.colour = palette[(ColourTheme::Colour)(extendedValue&7)];
                charAttr.colNum = (ColourTheme::Colour)(extendedValue&7);
                charAttr.colour_default = false;
                printf("bg colour %s", colName[charAttr.colNum]);
            }
            break;
        default:
            printf(" ** Extended Type %02X Not implemented **", extendedType);
    }
    printf("]");
    fflush(stdout);

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
    // Set field attribute flags
    glyph.at(pos)->setProtected((c>>5) & 1);
    glyph.at(pos)->setNumeric((c>>4) & 1);
    glyph.at(pos)->setDisplay((((c>>2)&3) != 3));
    glyph.at(pos)->setPenSelect((( c >> 2) & 3) == 2 || (( c >> 2) & 3) == 1);
    glyph.at(pos)->setIntensify(((c >> 2) & 3) == 2);
    glyph.at(pos)->setMDT(c & 1);
    glyph.at(pos)->setExtended(sfe);
    glyph.at(pos)->setFieldStart(true);

    // Field attributes do not have character attributes
    glyph.at(pos)->resetCharAttrs();

    // If it's not an Extended Field, set colours accordingly
    if (!sfe)
    {
        if (glyph.at(pos)->isProtected() && !glyph.at(pos)->isIntensify())
        {
            glyph.at(pos)->setColour(ColourTheme::PROTECTED_NORMAL);        /* Protected (Blue) */
        }
        else if (glyph.at(pos)->isProtected() && glyph.at(pos)->isIntensify())
        {
            glyph.at(pos)->setColour(ColourTheme::PROTECTED_INTENSIFIED);   /* Protected, Intensified (White) */
        }
        else if (!glyph.at(pos)->isProtected() && !glyph.at(pos)->isIntensify())
        {
            glyph.at(pos)->setColour(ColourTheme::UNPROTECTED_NORMAL);      /* Unprotected (Green) */
        }
        else
        {
            glyph.at(pos)->setColour(ColourTheme::UNPROTECTED_INTENSIFIED); /* Unrprotected, Intensified (Red) */
        }

        // If the display receives an SF order, it sets the associated extended
        // field attribute to its default value.
        glyph.at(pos)->setUScore(false);
        glyph.at(pos)->setReverse(false);
        glyph.at(pos)->setBlink(false);
    }

    QString attrs;

    if(!glyph.at(pos)->isProtected())
    {
        attrs = "unprot,";
    }
    else
    {
        attrs = "prot,";
    }
    if(glyph.at(pos)->isIntensify())
    {
        attrs.append("intens,");
    }
    if (glyph.at(pos)->isAutoSkip())
    {
        attrs.append("askip,");
    }
    if (!glyph.at(pos)->isDisplay())
    {
        attrs.append("nondisp,");
    }
    if (glyph.at(pos)->isPenSelect())
    {
        attrs.append("pen,");
    }
    if (glyph.at(pos)->isNumeric())
    {
        attrs.append("num,");
    }
    if (glyph.at(pos)->isMdtOn())
    {
        attrs.append("mdt,");
    }

    printf("%s", attrs.toLatin1().data());
    fflush(stdout);

/*    if (!sfe)
    {
        setFieldAttrs(pos);
    }
*/
}

void DisplayScreen::resetExtended(int pos)
{
    resetExtendedHilite(pos);

    glyph.at(pos)->setColour(ColourTheme::BLUE);

    glyph.at(pos)->setDisplay(true);
    glyph.at(pos)->setNumeric(false);
    glyph.at(pos)->setMDT(false);
    glyph.at(pos)->setPenSelect(false);
    glyph.at(pos)->setProtected(false);
}


void DisplayScreen::resetExtendedHilite(int pos)
{
    glyph.at(pos)->setUScore(false);
    glyph.at(pos)->setBlink(false);
    glyph.at(pos)->setReverse(false);
}

void DisplayScreen::setExtendedColour(int pos, bool foreground, unsigned char c)
{
    //TODO: Default colour?
    glyph.at(pos)->setColour((ColourTheme::Colour)(c&7));
//    glyph.at(pos)->setReverse(!foreground);
    if(foreground)
    {
        printf(" %s]", colName[glyph.at(pos)->getColour()]);
    }
}

void DisplayScreen::setExtendedBlink(int pos)
{
    glyph.at(pos)->setReverse(false);
    glyph.at(pos)->setBlink(true);
    printf("[Blink]");
}

void DisplayScreen::setExtendedReverse(int pos)
{
    glyph.at(pos)->setBlink(false);
    glyph.at(pos)->setReverse(true);
    printf("[Reverse]");
}

void DisplayScreen::setExtendedUscore(int pos)
{
    glyph.at(pos)->setUScore(true);
    printf("[UScore]");
}

void DisplayScreen::setFieldAttrs(int start)
{
    glyph.at(start)->setFieldStart(true);

    // resetFieldAttrs(start);

    glyph.at(start)->setText(IBM3270_CHAR_NULL);
    uscore.at(start)->setVisible(false);
}


int DisplayScreen::resetFieldAttrs(int start)
{

    int lastField = findField(start);

    int endPos = start + screenPos_max;

    for(int i = start; i < endPos; i++)
    {
        int offset = i % screenPos_max;

        if (glyph.at(offset)->isFieldStart() && i > start)
        {
            return lastField;
        }

        glyph.at(offset)->setProtected(glyph.at(lastField)->isProtected());
        glyph.at(offset)->setMDT(glyph.at(lastField)->isMdtOn());
        glyph.at(offset)->setNumeric(glyph.at(lastField)->isNumeric());
        glyph.at(offset)->setPenSelect(glyph.at(lastField)->isPenSelect());
        glyph.at(offset)->setDisplay(glyph.at(lastField)->isDisplay());

        glyph.at(offset)->setColour(glyph.at(lastField)->getColour());
        glyph.at(offset)->setUScore(glyph.at(lastField)->isUScore());
        glyph.at(offset)->setBlink(glyph.at(lastField)->isBlink());
        glyph.at(offset)->setReverse(glyph.at(lastField)->isReverse());

        glyph.at(offset)->resetCharAttrs();

        if (glyph.at(offset)->isDisplay() & !glyph.at(offset)->isFieldStart())
        {
            if (glyph.at(offset)->isReverse())
            {
                cell.at(offset)->setBrush(palette[glyph.at(offset)->getColour()]);
                glyph.at(offset)->setBrush(palette[ColourTheme::BLACK]);
            }
            else
            {
                cell.at(offset)->setBrush(palette[ColourTheme::BLACK]);
                glyph.at(offset)->setBrush(palette[glyph.at(offset)->getColour()]);
            }
            if (glyph.at(offset)->isUScore())
            {
                uscore.at(offset)->setPen(QPen(palette[glyph.at(offset)->getColour()],0));
            }
            else
            {
                uscore.at(offset)->setVisible(false);
            }
        }
        else
        {
            cell.at(offset)->setBrush(palette[ColourTheme::BLACK]);
            glyph.at(offset)->setBrush(palette[ColourTheme::BLACK]);
        }
    }

    return lastField;
}

void DisplayScreen::cascadeAttrs(int start)
{
    int endPos = start + screenPos_max;

    for(int i = start; i < endPos; i++)
    {
        int offset = i % screenPos_max;

        if (glyph.at(offset)->isFieldStart() && i > start)
        {
            return;
        }

        glyph.at(offset)->setProtected(glyph.at(start)->isProtected());
        glyph.at(offset)->setMDT(glyph.at(start)->isMdtOn());
        glyph.at(offset)->setNumeric(glyph.at(start)->isNumeric());
        glyph.at(offset)->setPenSelect(glyph.at(start)->isPenSelect());
        glyph.at(offset)->setDisplay(glyph.at(start)->isDisplay());

        glyph.at(offset)->setColour(glyph.at(start)->getColour());
        glyph.at(offset)->setUScore(glyph.at(start)->isUScore());
        glyph.at(offset)->setBlink(glyph.at(start)->isBlink());
        glyph.at(offset)->setReverse(glyph.at(start)->isReverse());

        glyph.at(offset)->resetCharAttrs();

        if (glyph.at(offset)->isDisplay() & !glyph.at(offset)->isFieldStart())
        {
            if (glyph.at(offset)->isReverse())
            {
                cell.at(offset)->setBrush(palette[glyph.at(offset)->getColour()]);
                glyph.at(offset)->setBrush(palette[ColourTheme::BLACK]);
            }
            else
            {
                cell.at(offset)->setBrush(palette[ColourTheme::BLACK]);
                glyph.at(offset)->setBrush(palette[glyph.at(offset)->getColour()]);
            }
            if (glyph.at(offset)->isUScore())
            {
                uscore.at(offset)->setPen(QPen(palette[glyph.at(offset)->getColour()],0));
            }
            else
            {
                uscore.at(offset)->setVisible(false);
            }
        }
        else
        {
            cell.at(offset)->setBrush(palette[ColourTheme::BLACK]);
            glyph.at(offset)->setBrush(palette[ColourTheme::BLACK]);
        }
    }

}

/* Reset all MDTs in the display; it's probably faster to just loop through the entire buffer
 * rather than calling findNextField()
 *
 */
void DisplayScreen::resetMDTs()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        if (glyph.at(i)->isFieldStart() & glyph.at(i)->isMdtOn())
        {
            glyph.at(i)->setMDT(false);
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
    if (glyph.at(pos)->isProtected() || glyph.at(pos)->isFieldStart())
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
        printf("This Field at: %d,%d, next field at %d,%d - last byte of this field %02X\n", (int)(thisField/screen_x), (int)(thisField-((int)(thisField/screen_x)*screen_x)), (int)(nextField/screen_x), (int)(nextField-((int)(nextField/screen_x)*screen_x)), glyph.at(nextField - 1)->getEBCDIC() );
        uchar lastChar = glyph.at(nextField - 1)->getEBCDIC();
        if (lastChar != IBM3270_CHAR_NULL && lastChar != IBM3270_CHAR_SPACE)
        {
            // Insert not okay
        }
        int endPos = -1;
        for(int i = pos; i < (pos + screenPos_max); i++)
        {
            int offset = i % screenPos_max;
            if (glyph.at(offset)->isProtected() || glyph.at(offset)->isFieldStart())
            {
                break;
            }
            if (glyph.at(offset)->getEBCDIC() == IBM3270_CHAR_NULL)
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

        bool tmpGE = geActive;
        for(int fld = endPos; fld > pos; fld--)
        {
            int offset = fld % screenPos_max;
            int offsetPrev = (fld - 1) % screenPos_max;

            //TODO: Improve performance
            glyph.at(offset)->setFieldStart(glyph.at(offsetPrev)->isFieldStart());

            glyph.at(offset)->setProtected(glyph.at(offsetPrev)->isProtected());
            glyph.at(offset)->setMDT(glyph.at(offsetPrev)->isMdtOn());
            glyph.at(offset)->setNumeric(glyph.at(offsetPrev)->isNumeric());
            glyph.at(offset)->setPenSelect(glyph.at(offsetPrev)->isPenSelect());
            glyph.at(offset)->setDisplay(glyph.at(offsetPrev)->isDisplay());

            /* NOTE: These should already have been set for the entire field. Not convinced they
             * need to move.. */
            glyph.at(offset)->setColour(glyph.at(offsetPrev)->getColour());
            glyph.at(offset)->setUScore(glyph.at(offsetPrev)->isUScore());
            glyph.at(offset)->setBlink(glyph.at(offsetPrev)->isBlink());
            glyph.at(offset)->setReverse(glyph.at(offsetPrev)->isReverse());

            /* Character attributes move with the insert */
            glyph.at(offset)->setCharAttrs(glyph.at(offsetPrev)->hasCharAttrs(Glyph::CharAttr::EXTENDED), Glyph::CharAttr::EXTENDED);
            glyph.at(offset)->setCharAttrs(glyph.at(offsetPrev)->hasCharAttrs(Glyph::CharAttr::COLOUR), Glyph::CharAttr::COLOUR);
            glyph.at(offset)->setCharAttrs(glyph.at(offsetPrev)->hasCharAttrs(Glyph::CharAttr::CHARSET), Glyph::CharAttr::CHARSET);
            glyph.at(offset)->setCharAttrs(glyph.at(offsetPrev)->hasCharAttrs(Glyph::CharAttr::TRANSPARANCY), Glyph::CharAttr::TRANSPARANCY);

            geActive = glyph.at(offsetPrev)->isGraphic();
            setChar(offset, glyph.at(offsetPrev)->getEBCDIC(), true, false);
        }
        geActive = tmpGE;
    }

    glyph.at(thisField)->setMDT(true);

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
    return glyph.at(pos)->isAutoSkip();
}

bool DisplayScreen::isProtected(int pos)
{
    return glyph.at(pos)->isProtected();
}

bool DisplayScreen::isFieldStart(int pos)
{
    return glyph.at(pos)->isFieldStart();
}

void DisplayScreen::deleteChar(int pos)
{
    if (glyph.at(pos)->isProtected())
    {
        printf("Protected!\n");
        fflush(stdout);
        return;
    }

    int endPos = findNextField(pos);

    for(int fld = pos; fld < endPos - 1 && glyph.at(fld % screenPos_max)->getEBCDIC() != IBM3270_CHAR_NULL; fld++)
    {
        int offset = fld % screenPos_max;
        int offsetNext = (fld + 1) % screenPos_max;

        glyph.at(offset)->setFieldStart(glyph.at(offsetNext)->isFieldStart());

        glyph.at(offset)->setProtected(glyph.at(offsetNext)->isProtected());
        glyph.at(offset)->setMDT(glyph.at(offsetNext)->isMdtOn());
        glyph.at(offset)->setNumeric(glyph.at(offsetNext)->isNumeric());
        glyph.at(offset)->setPenSelect(glyph.at(offsetNext)->isPenSelect());
        glyph.at(offset)->setDisplay(glyph.at(offsetNext)->isDisplay());

        glyph.at(offset)->setColour(glyph.at(offsetNext)->getColour());
        glyph.at(offset)->setUScore(glyph.at(offsetNext)->isUScore());
        glyph.at(offset)->setBlink(glyph.at(offsetNext)->isBlink());
        glyph.at(offset)->setReverse(glyph.at(offsetNext)->isReverse());

        glyph.at(offset)->setCharAttrs(glyph.at(offsetNext)->hasCharAttrs(Glyph::CharAttr::EXTENDED), Glyph::CharAttr::EXTENDED);
        glyph.at(offset)->setCharAttrs(glyph.at(offsetNext)->hasCharAttrs(Glyph::CharAttr::COLOUR), Glyph::CharAttr::COLOUR);
        glyph.at(offset)->setCharAttrs(glyph.at(offsetNext)->hasCharAttrs(Glyph::CharAttr::CHARSET), Glyph::CharAttr::CHARSET);
        glyph.at(offset)->setCharAttrs(glyph.at(offsetNext)->hasCharAttrs(Glyph::CharAttr::TRANSPARANCY), Glyph::CharAttr::TRANSPARANCY);

        bool tmpGE = geActive;
        setChar(offset, glyph.at(offsetNext)->getEBCDIC(), true, false);
        geActive = tmpGE;
    }

    glyph.at(endPos - 1)->setText(IBM3270_CHAR_NULL);
    glyph.at(findField(pos))->setMDT(true);
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
        glyph.at(i % screenPos_max)->setText(0x00);
    }

    glyph.at(findField(pos))->setMDT(true);
}

void DisplayScreen::eraseUnprotected(int start, int end)
{
    if (end < start)
    {
        end += screenPos_max;
    }

    int thisField = findField(start);
    if (glyph.at(thisField)->isProtected())
    {
        start = findNextUnprotectedField(start);
    }

    for(int i = start; i < end; i++)
    {
        if(glyph.at(i)->isProtected() || glyph.at(i)->isFieldStart())
        {
            i = findNextUnprotectedField(i);
        }
        else
        {
                glyph.at(i)->setText(IBM3270_CHAR_SPACE);
        }
    }
}

void DisplayScreen::setCursorColour(bool inherit)
{
    cursorColour = inherit;
    if (inherit)
    {
        Glyph *g = (Glyph *)cursor.parentItem()->childItems()[0];
        cursor.setBrush(palette[g->getColour()]);
    }
    else
    {
        cursor.setBrush(QBrush(QColor(0xBB, 0xBB, 0xBB)));
    }
}

void DisplayScreen::setCursor(int pos)
{
    cursor.setParentItem(cell.at(pos));
    cursor.setBrush(palette[glyph.at(pos)->getColour()]);
    cursor.setPos(cell.at(pos)->boundingRect().left(), cell.at(pos)->boundingRect().top());
}

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
        if (glyph.at(i)->isBlink())
        {
            if (blinkShow)
            {
                glyph.at(i)->setBrush(palette[glyph.at(i)->getColour()]);
            }
            else
            {
                glyph.at(i)->setBrush(palette[ColourTheme::BLACK]);
            }
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

        if (glyph.at(offset)->isFieldStart())
        {
            return offset;
        }
    }
    return pos;
}

int DisplayScreen::findNextField(int pos)
{
    if(glyph.at(pos)->isFieldStart())
    {
        pos++;
    }
    int tmpPos;
    for(int i = pos; i < (pos + screenPos_max); i++)
    {
        tmpPos = i % screenPos_max;
        if (glyph.at(tmpPos)->isFieldStart())
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
        // Check this position for unprotected and fieldStart and check the position for
        // fieldStart - an unprotected field cannot start where two fieldStarts are adajacent
        tmpPos = i % screenPos_max;
        tmpNxt = (i + 1) % screenPos_max;
        if (glyph.at(tmpPos)->isFieldStart() && !glyph.at(tmpPos)->isProtected() && !glyph.at(tmpNxt)->isFieldStart())
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
        tmpNxt = (tmpPos + 1) % screenPos_max;
        if (glyph.at(tmpPos)->isFieldStart() && !glyph.at(tmpPos)->isProtected() && !glyph.at(tmpNxt)->isFieldStart())
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
        if (glyph.at(i)->isFieldStart() && !glyph.at(i)->isProtected())
        {
            int firstField = i;
            int thisField = i;
            do
            {
                if (glyph.at(thisField)->isMdtOn() && !glyph.at(thisField)->isProtected())
                {
                    buffer.append(IBM3270_SBA);

                    int nextPos = (thisField + 1) % screenPos_max;

                    addPosToBuffer(buffer, nextPos);

                    do
                    {
                        uchar b = glyph.at(thisField++)->getEBCDIC();
                        if (b != IBM3270_CHAR_NULL)
                        {
                            buffer.append(b);
                        }
                        thisField = thisField % screenPos_max;
                        //FIXME: Not sure this is right. This was a quick hack to cater for there being only one unprotected
                        //       field on the screen.
                        if (thisField == i)
                        {
                            printf("Wrapped!");
                            return;
                        }
                    }
                    while(!glyph.at(thisField)->isFieldStart());
                }
                thisField = findNextField(thisField);
            }
            while(thisField > firstField);
            return;
        }
    }
}

/**
 * @brief DisplayScreen::addPosToBuffer
 *        Utility method to insert 'pos' into 'buffer' as two bytes, doubling 0xFF if needed.
 * @param buffer
 * @param pos
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

void DisplayScreen::dumpFields()
{
    printf("Screen_X = %d, screen_Y =%d\n", screen_x, screen_y);
    fflush(stdout);
    for(int i = 0; i < screenPos_max; i++)
    {
        if (glyph.at(i)->isFieldStart())
        {
            int tmpy = i / screen_x;
            int tmpx = i - (tmpy * screen_x);

            printf("Field at %4d (%2d,%2d) : Prot: %d\n", i, tmpx, tmpy, glyph.at(i)->isProtected());
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
        ascii.append(glyph.at(i)->text());
        printf("%2.2X ", glyph.at(i)->getEBCDIC());
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
    printf("    Character: %s (hex %2.2X EBCDIC %2.2X)", glyph.at(pos)->text().toLatin1().data(),glyph.at(pos)->text().toLatin1().at(0),glyph.at(pos)->getEBCDIC());
    printf("    Field : %d\n    MDT: %d\n    Protected: %d\n    Numeric: %d\n    Autoskip: %d\n    Display: %d\n",
                glyph.at(pos)->isFieldStart(),
                glyph.at(pos)->isMdtOn(),
                glyph.at(pos)->isProtected(),
                glyph.at(pos)->isNumeric(),
                glyph.at(pos)->isAutoSkip(),
                glyph.at(pos)->isDisplay());
    printf("    Extended: %d\n    Intensify: %d\n    UScore: %d\n    Reverse: %d\n    Blink: %d\n",
                glyph.at(pos)->isExtended(),
                glyph.at(pos)->isIntensify(),
                glyph.at(pos)->isUScore(),
                glyph.at(pos)->isReverse(),
                glyph.at(pos)->isBlink());
    printf("    Character Attributes: Extended %d CharSet %d Colour %d\n    Colour: %d\n    Graphic: %d\n",
                glyph.at(pos)->hasCharAttrs(Glyph::EXTENDED),
                glyph.at(pos)->hasCharAttrs(Glyph::CHARSET),
                glyph.at(pos)->hasCharAttrs(Glyph::COLOUR),
                glyph.at(pos)->getColour(),
                glyph.at(pos)->isGraphic());

    fflush(stdout);

}

void DisplayScreen::getScreen(QByteArray &buffer)
{
    dumpDisplay();

    for (int i = 0; i < screenPos_max; i++)
    {
        if (glyph.at(i)->isFieldStart())
        {
            buffer.append(IBM3270_SF);
            uchar attr;
            if (glyph.at(i)->isDisplay() & !glyph.at(i)->isPenSelect())
            {
                attr = 0x00;
            }
            else if (glyph.at(i)->isDisplay() & glyph.at(i)->isPenSelect())
            {
                attr = 0x01;
            }
            else if(glyph.at(i)->isIntensify())
            {
                attr = 0x10;
            }
            else
            {
                attr = 0x11;
            }

            int byte = twelveBitBufferAddress[glyph.at(i)->isMdtOn() | attr << 3 | glyph.at(i)->isNumeric() << 4 | glyph.at(i)->isProtected() << 5];

            buffer.append(byte);

            // Double up 0xFF bytes
            if (byte == 0xFF)
            {
                buffer.append(byte);
            }
        }
        else
        {
            buffer.append(glyph.at(i)->getEBCDIC());
        }
    }
}
