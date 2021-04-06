#include "DisplayScreen.h"

DisplayScreen::DisplayScreen(int screen_x, int screen_y)
{
    setColourPalette(default_palette);

    setBackgroundBrush(palette[0]);

    this->screen_x = screen_x;
    this->screen_y = screen_y;

    gridSize_X = 50;
    gridSize_Y = 50;

    screenPos_max = screen_x * screen_y;

    // Default settings
    fontScaling = true;
    ruler = false;
    blinkShow = false;
    cursorShow = true;

    attrs = new Attributes[screenPos_max];

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
            cell.at(pos)->setPen(Qt::NoPen);
            cell.at(pos)->setPos(x_pos, y_pos);

            glyph.replace(pos, new Text(x_pos, y_pos, cell.at(pos)));
            glyph.at(pos)->setFlag(QGraphicsItem::ItemIsSelectable);

            uscore.replace(pos, new QGraphicsLineItem(0, 0, gridSize_X, 0));
            uscore.at(pos)->setZValue(1);
            uscore.at(pos)->setPos(x_pos, y_pos + gridSize_Y);

            addItem(uscore.at(pos));
            addItem(cell.at(pos));
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
    int statusPos = (screen_y * gridSize_Y);

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
    delete attrs;
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

    if (fontScaling)
    {
        QFontMetrics fm = QFontMetrics(font);
//        QRectF boxRect = QRectF(0, 0, fm->maxWidth(), fm->lineSpacing() * 0.99);
        QRectF charBounds = fm.boundingRect("┼");
        QRectF boxRect = QRectF(0, 0, fm.width("┼", 1) - 1, fm.lineSpacing() - 5);

        printf("DisplayScreen   : charBounds (┼) =  %f x %f\n   boxRect = %f x %f\n", charBounds.x(), charBounds.y(), boxRect.width(), boxRect.height());
        printf("Font Width (┼)        : %d\n",fm.width("┼"));
        printf("Font Height (┼)       : %d\n",fm.height());
        printf("Font Line Spacing (┼) : %d\n",fm.lineSpacing());
        printf("Font Max Width        : %d\n", fm.maxWidth());
        printf("Font Descent          : %d\n", fm.descent());
        printf("Font Ascent           : %d\n", fm.ascent());

        fflush(stdout);

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

void DisplayScreen::setColourPalette(QColor *c)
{
    for (int i = 0; i < 12; i++)
    {
        palette[i] = c[i];
    }
}

void DisplayScreen::resetColours()
{
    for (int i = 0; i < screenPos_max; i++)
    {
        if (attrs[i].reverse)
        {
            cell.at(i)->setBrush(palette[attrs[i].colNum]);
            glyph.at(i)->setBrush(palette[0]);
            printf("<reverse>");
        }
        else
        {
            glyph.at(i)->setBrush(palette[attrs[i].colNum]);
            cell.at(i)->setBrush(palette[0]);
        }
        if (!attrs[i].display)
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
        cell.at(i)->setBrush(palette[0]);

        uscore.at(i)->setVisible(false);

        glyph.at(i)->setBrush(palette[1]);
        glyph.at(i)->setText(0x00, 0x00, false);

        attrs[i].colNum = 1;

        attrs[i].askip = false;
        attrs[i].num = false;
        attrs[i].mdt = false;
        attrs[i].prot = false;

        attrs[i].fieldStart = false;

        attrs[i].display = true;
        attrs[1].pen = false;
        attrs[i].intensify = false;

        attrs[i].extended = false;
        attrs[i].uscore = false;

        attrs[i].reverse = false;
        attrs[i].blink = false;

        attrs[i].charAttr = false;

    }
    resetCharAttr();

    geActive = false;
}

void DisplayScreen::setChar(int pos, short unsigned int c, bool move)
{

    int lastField;

    if (attrs[pos].fieldStart)
    {
        attrs[pos].fieldStart = false;
        lastField = resetFieldAttrs(pos);
    }
    else
    {
        lastField = findField(pos);
    }

    attrs[pos].charAttr = useCharAttr;

    if(!geActive)
    {
        glyph.at(pos)->setText(EBCDICtoASCIImap[c], c, false);
    }
    else
    {
        glyph.at(pos)->setText(EBCDICtoASCIImapge[c], c, true);
        geActive = false;
    }

    if (!move)
    {
        if (attrs[pos].charAttr)
        {
            // Set colour

            if (!charAttr.colour_default)
            {
                attrs[pos].colNum = charAttr.colNum;
                printf("<CH %s>", colName[attrs[pos].colNum]);
            }
            else
            {
                attrs[pos].colNum = attrs[lastField].colNum;
            }

            // Reverse video
            if (!charAttr.reverse_default)
            {
                attrs[pos].reverse = charAttr.reverse;
                if (attrs[pos].reverse)
                {
                    printf("<CH reverse>");
                }
            }
            else
            {
                attrs[pos].reverse = attrs[lastField].reverse;
            }

            // Underscore
            if (!charAttr.uscore_default)
            {
                attrs[pos].uscore = charAttr.uscore;
                if (attrs[pos].uscore)
                {
                    printf("<CH uscore>");
                }
            }
            else
            {
                attrs[pos].uscore = attrs[lastField].uscore;
            }
        }
        else
        {
            attrs[pos].uscore = attrs[lastField].uscore;
            attrs[pos].reverse = attrs[lastField].reverse;
            attrs[pos].colNum = attrs[lastField].colNum;
        }
    }

    // Colour - non-display / reverse / normal
    if (!attrs[pos].display)
    {
        glyph.at(pos)->setBrush(cell.at(pos)->brush());
    }
    else
    {
        if (attrs[pos].reverse)
        {
            cell.at(pos)->setBrush(palette[attrs[pos].colNum]);
            glyph.at(pos)->setBrush(palette[0]);
            printf("<reverse>");
        }
        else
        {
            glyph.at(pos)->setBrush(palette[attrs[pos].colNum]);
            cell.at(pos)->setBrush(palette[0]);
        }
    }

    //
    // Underscore processing
    //

    if (attrs[pos].uscore)
    {
        uscore.at(pos)->setVisible(true);
        uscore.at(pos)->setPen(QPen(palette[attrs[pos].colNum],0));
 //       uscore[pos]->pen().setCosmetic(true);
 //       uscore[pos]->pen().setColor(palette[attrs[pos].colNum]);

        printf("<uscore %d>", attrs[pos].colNum);
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
            charAttr.colour = palette[extendedValue&7];
            charAttr.colNum = extendedValue&7;
            charAttr.colour_default = false;
            printf("fg colour %s", colName[charAttr.colNum]);
            break;
        case IBM3270_EXT_BG_COLOUR:
            charAttr.colour = palette[extendedValue&7];
            charAttr.colNum = extendedValue&7;
            charAttr.colour_default = false;
            printf("bg colour %s", colName[charAttr.colNum]);
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
}

void DisplayScreen::setGraphicEscape()
{
    geActive = true;
}

void DisplayScreen::setField(int pos, unsigned char c, bool sfe)
{
    printf("3270 Attribute %2.2X at %d", c, pos);

    attrs[pos].prot = (c>>5)&1;
    attrs[pos].num  = (c>>4)&1;
    attrs[pos].display = (((c>>2)&3) != 3);
    attrs[pos].pen = (( c >> 2) & 3) == 2 || (( c >> 2) & 3) == 1;
    attrs[pos].intensify = ((c >> 2) & 3) == 2;
    attrs[pos].mdt = c & 1;
    attrs[pos].extended = sfe;

    attrs[pos].charAttr = false;
    attrs[pos].askip = (attrs[pos].prot & attrs[pos].num);

    //    printf("P:%d N:%d D:%d L:%d I:%d M:%d A:%d\n", f.prot, f.num, f.display, f.pen, f.intensify, f.mdt, f.askip);

    if (!sfe)
    {
        if (attrs[pos].prot && !attrs[pos].intensify)
        {
            attrs[pos].colNum = 8;  /* Protected (Blue) */
        }
        else if (attrs[pos].prot && attrs[pos].intensify)
        {
            attrs[pos].colNum = 11;  /* Protected, Intensified (White) */
        }
        else if (!attrs[pos].prot && !attrs[pos].intensify)
        {
            attrs[pos].colNum = 10;  /* Unprotected (Green) */
        }
        else
        {
            attrs[pos].colNum = 9;  /* Unrprotected, Intensified (Red) */
        }

        attrs[pos].uscore = false;
        attrs[pos].reverse = false;
        attrs[pos].blink = false;
    }

    if(!attrs[pos].prot)
    {
        printf("(unprot,");
    }
    else
    {
        printf("(prot,");
    }
    if(attrs[pos].intensify)
    {
        printf("intens,");
    }
    if (attrs[pos].askip)
    {
        printf("askip,");
    }
    if (!attrs[pos].display)
    {
        printf("nondisp,");
    }
    if (attrs[pos].pen)
    {
        printf("pen,");
    }
    if (attrs[pos].num)
    {
        printf("num,");
    }
    if (attrs[pos].mdt)
    {
        printf("mdt,");
    }
    printf(")");
    fflush(stdout);

    if (!sfe)
    {
        setFieldAttrs(pos);
    }

//    printf("[S%4d - %2d,%2d): Attribute byte: %2.2X prot=%d num=%d display=%d pen=%d mdt=%d reverse=%d intens=%d : ", pos, px, py, c, attrs[pos].prot, attrs[pos].num, attrs[pos].display, attrs[pos].pen, attrs[pos].mdt, attrs[pos].reverse, attrs[pos].intensify);

}

void DisplayScreen::resetExtended(int pos)
{
    resetExtendedHilite(pos);

    attrs[pos].colNum = 1;

    attrs[pos].display = true;
    attrs[pos].num = false;
    attrs[pos].mdt = false;
    attrs[pos].pen = false;
    attrs[pos].askip = false;
    attrs[pos].prot = false;
}


void DisplayScreen::resetExtendedHilite(int pos)
{
    attrs[pos].uscore = false;
    attrs[pos].blink = false;
    attrs[pos].reverse = false;
}

void DisplayScreen::setExtendedColour(int pos, bool foreground, unsigned char c)
{
    attrs[pos].colNum = c&7;
    attrs[pos].reverse = !foreground;
    if(foreground)
    {
        printf(" %s]", colName[attrs[pos].colNum]);
    }
}

void DisplayScreen::setExtendedBlink(int pos)
{
    attrs[pos].reverse = false;
    attrs[pos].blink = true;
    printf("[Blink]");
}

void DisplayScreen::setExtendedReverse(int pos)
{
    attrs[pos].blink = false;
    attrs[pos].reverse = true;
    printf("[Reverse]");
}

void DisplayScreen::setExtendedUscore(int pos)
{
    attrs[pos].uscore = true;
    printf("[UScore]");
}

void DisplayScreen::setFieldAttrs(int start)
{
    attrs[start].fieldStart = true;

    printf("[setting field %d to uscore %d colour %s]", start, attrs[start].uscore, colName[attrs[start].colNum]);
    fflush(stdout);

    resetFieldAttrs(start);

    //TODO should store the field attributes?
    glyph.at(start)->setText(IBM3270_CHAR_NULL, IBM3270_CHAR_NULL, false);
    uscore.at(start)->setVisible(false);
}


int DisplayScreen::resetFieldAttrs(int start)
{
    int lastField = findField(start);

    printf("[attributes obtained from %d]", lastField);

    int endPos = start + screenPos_max;

    for(int i = start; i < endPos; i++)
    {
        int offset = i % screenPos_max;

        if (attrs[offset].fieldStart && i > start)
        {
            printf("[ended at %d]", offset);
            return lastField;
        }

        attrs[offset].prot = attrs[lastField].prot;
        attrs[offset].mdt = attrs[lastField].mdt;
        attrs[offset].num = attrs[lastField].num;
        attrs[offset].askip = attrs[lastField].askip;
        attrs[offset].pen = attrs[lastField].pen;
        attrs[offset].display = attrs[lastField].display;

        attrs[offset].colNum = attrs[lastField].colNum;
        attrs[offset].uscore = attrs[lastField].uscore;
        attrs[offset].blink = attrs[lastField].blink;
        attrs[offset].reverse = attrs[lastField].reverse;
        attrs[offset].charAttr = false;

        if (attrs[offset].display)
        {
            if (attrs[offset].reverse)
            {
                cell.at(offset)->setBrush(palette[attrs[offset].colNum]);
                glyph.at(offset)->setBrush(palette[0]);
            }
            else
            {
                cell.at(offset)->setBrush(palette[0]);
                glyph.at(offset)->setBrush(palette[attrs[offset].colNum]);
            }
            if (attrs[offset].uscore)
            {
                uscore.at(offset)->setPen(QPen(palette[attrs[offset].colNum],0));
//                uscore[offset]->pen().setCosmetic(true);
//                uscore[offset]->pen().setColor(palette[attrs[offset].colNum]);
            }
            else
            {
                uscore.at(offset)->setVisible(false);
            }
        }
        else
        {
            cell.at(offset)->setBrush(palette[0]);
            glyph.at(offset)->setBrush(palette[0]);
        }
    }

    printf("[ended at %d]", endPos);
    return lastField;
}


bool DisplayScreen::insertChar(int pos, unsigned char c, bool insertMode)
{
    if (attrs[pos].prot || attrs[pos].fieldStart)
    {
        printf("Protected!\n");
        fflush(stdout);
        return false;
    }

    int thisField = findField(pos);

    if (insertMode)
    {
        int endPos = -1;
        for(int i = pos; i < (pos + screenPos_max); i++)
        {
            int offset = i % screenPos_max;
            if (attrs[offset].prot || attrs[offset].fieldStart)
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
        printf("Field length: %d, starting at %d, ending at %d\n", endPos - pos, pos, endPos);
        fflush(stdout);
        bool tmpGE = geActive;
        for(int fld = endPos; fld > pos; fld--)
        {
            int offset = fld % screenPos_max;
            int offsetPrev = (fld - 1) % screenPos_max;

//            printf("Moving %c to %d\n", glyph[offsetPrev]->getEBCDIC(), offset);
//            fflush(stdout);
            attrs[offset] = attrs[offsetPrev];
            geActive = glyph.at(offsetPrev)->getGraphic();
            setChar(offset, glyph.at(offsetPrev)->getEBCDIC(), true);
        }
        geActive = tmpGE;
    }

    printf("MDT set for %d\n", thisField);
    attrs[thisField].mdt = true;
    setChar(pos, ASCIItoEBCDICmap[c], false);

    return true;
}

/**
 * \class DisplayScreen::isAskip
 *
 * \brief isAskip returns a boolean indicating whether the supplied screen position contains askip.
 */
bool DisplayScreen::isAskip(int pos)
{
    return attrs[pos].askip;
}

void DisplayScreen::deleteChar(int pos)
{
    if (attrs[pos].prot)
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

        attrs[offset] = attrs[offsetNext];
        bool tmpGE = geActive;
        setChar(offset, glyph.at(offsetNext)->getEBCDIC(), true);
        geActive = tmpGE;
    }

    glyph.at(endPos - 1)->setText(IBM3270_CHAR_NULL, IBM3270_CHAR_NULL, false);
    attrs[findField(pos)].mdt = true;
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
        glyph.at(i % screenPos_max)->setText(0x00, 0x00, false);
    }

    attrs[findField(pos)].mdt = true;
}

void DisplayScreen::eraseUnprotected(int start, int end)
{
    if (end < start)
    {
        end += screenPos_max;
    }

    int thisField = findField(start);
    if (attrs[thisField].prot)
    {
        start = findNextUnprotectedField(start);
    }

    for(int i = start; i < end; i++)
    {
        if(attrs[i].prot || attrs[i].fieldStart)
        {
            i = findNextUnprotectedField(i);
        }
        else
        {
                glyph.at(i)->setText(" ", IBM3270_CHAR_SPACE, false);
        }
    }
}

void DisplayScreen::setCursor(int pos)
{
    cursor.setParentItem(cell.at(pos));
    cursor.setBrush(palette[attrs[pos].colNum]);
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

void DisplayScreen::toggleRuler()
{
    ruler = !ruler;

    if (ruler)
    {
        crosshair_X.show();
        crosshair_Y.show();
    }
    else
    {
        crosshair_X.hide();
        crosshair_Y.hide();
    }
}

void DisplayScreen::drawRuler(int x, int y)
{
    if (ruler)
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
        if (attrs[i].blink)
        {
            if (blinkShow)
            {
                glyph.at(i)->setBrush(palette[attrs[i].colNum]);
            }
            else
            {
                glyph.at(i)->setBrush(palette[0]);
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
//    printf("findField: endpos = %d\n", endPos);
//    fflush(stdout);
    for (int i = pos; i > endPos ; i--)
    {
        int offset = i;
        if (i < 0)
        {
            offset = screenPos_max + i;
        }

//        printf("findField: i = %d, offset = %d (new offset = %d)\n", i, +(i%(SCREENX*SCREENY)), offset);
//        fflush(stdout);
        if (attrs[offset].fieldStart)
        {
            return offset;
        }
    }
    return pos;
}

int DisplayScreen::findNextField(int pos)
{
    if(attrs[pos].fieldStart)
    {
        pos++;
    }
    int tmpPos;
    for(int i = pos; i < (pos + screenPos_max); i++)
    {
        tmpPos = i % screenPos_max;
        if (attrs[tmpPos].fieldStart)
        {
            return tmpPos;
        }
    }
//    printf("No next field found: start = %d, end = %d\n", pos, pos +(SCREENX * SCREENY));
    fflush(stdout);
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
        if (attrs[tmpPos].fieldStart && !attrs[tmpPos].prot && !attrs[tmpNxt].fieldStart)
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
        if (attrs[tmpPos].fieldStart && !attrs[tmpPos].prot && !attrs[tmpNxt].fieldStart)
        {
            return tmpPos;
        }
    }
    printf("No unprotected field found: start = %d, end = %d\n", pos, pos + screenPos_max);
    fflush(stdout);
    return pos - 1;
}

void DisplayScreen::getModifiedFields(QByteArray &buffer)
{
    for(int i = 0; i < screenPos_max; i++)
    {
        if (attrs[i].fieldStart && !attrs[i].prot)
        {
            int firstField = i;
            int thisField = i;
            do
            {
                //TODO: 0xFF may occur, and need to be doubled
                if (attrs[thisField].mdt && !attrs[thisField].prot)
                {
                    buffer.append(IBM3270_SBA);

                    printf("Adding field at %d : ", thisField);

                    int nextPos = (thisField + 1) % screenPos_max;

                    if (nextPos < 4096) // 12 bit
                    {
                        buffer.append(twelveBitBufferAddress[(nextPos>>6)&63]);
                        buffer.append(twelveBitBufferAddress[(nextPos&63)]);
                    }
                    else if (nextPos < 16384) // 14 bit
                    {
                        buffer.append((nextPos>>8)&63);
                        buffer.append(nextPos&0xFF);
                    }
                    else // 16 bit
                    {
                        buffer.append((nextPos>>8)&0xFF);
                        buffer.append(nextPos&0xFF);
                    }


                    attrs[thisField].mdt = false;

                    do
                    {
                        uchar b = glyph.at(thisField++)->getEBCDIC();
                        if (b != IBM3270_CHAR_NULL)
                        {
                            buffer.append(b);
                            printf("%s", glyph.at(thisField-1)->text().toLatin1().data());
                        }
                        thisField = thisField % screenPos_max;
                    }
                    while(!attrs[thisField].fieldStart);
                    printf("\n");
                    fflush(stdout);
                }
                thisField = findNextField(thisField);
            }
            while(thisField > firstField);
            return;
        }
    }
}

void DisplayScreen::dumpFields()
{
    printf("Screen_X = %d, screen_y =%d\n", screen_x, screen_y);
    fflush(stdout);
    for(int i = 0; i < screenPos_max; i++)
    {
        if (attrs[i].fieldStart)
        {
            int tmpy = i / screen_x;
            int tmpx = i - (tmpy * screen_x);

            printf("Field at %4d (%2d,%2d) : Prot: %d\n", i, tmpx, tmpy, attrs[i].prot);
        }
    }
    fflush(stdout);
}


void DisplayScreen::dumpDisplay()
{
    printf("---- SCREEN ----");
    for (int i = 0; i < screenPos_max; i++)
    {
        if (i % screen_x == 0)
        {
            printf("\n");
        }
        printf("%2.2X ", glyph.at(i)->getEBCDIC());
    }
    printf("\n---- SCREEN ----\n");
    fflush(stdout);
}

void DisplayScreen::dumpAttrs(int pos)
{
    printf("   Attrs: Prot:%d Ext:%d Start:%d Skip:%d Display:%d Uscore:%d Rev:%d Blnk:%d Intens:%d Num:%d Pen:%d\n",
           attrs[pos].prot, attrs[pos].extended, attrs[pos].fieldStart, attrs[pos].askip, attrs[pos].display, attrs[pos].uscore, attrs[pos].reverse, attrs[pos].blink, attrs[pos].intensify, attrs[pos].num, attrs[pos].pen);
}
