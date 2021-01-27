#include "DisplayScreen.h"

DisplayScreen::DisplayScreen(int screen_x, int screen_y)
{
    setBackgroundBrush(palette[0]);

    this->screen_x = screen_x;
    this->screen_y = screen_y;

    gridSize_X = 50;
    gridSize_Y = 50;

    screenPos_max = screen_x * screen_y;

    line.setCosmetic(true);
    line.setWidth(1);

    attrs = new Attributes[screenPos_max];
    glyph = new Text*[screenPos_max];
    cell = new QGraphicsRectItem*[screenPos_max];
    uscore = new QGraphicsLineItem*[screenPos_max];

    fontScaling = true;

    for(int y = 0; y < screen_y; y++)
    {
        qreal y_pos = y * gridSize_Y;

        for(int x = 0; x < screen_x; x++)
        {
            int pos = x + (y * screen_x);

            qreal x_pos = x * gridSize_X;

            cell[pos] = new QGraphicsRectItem(0, 0, gridSize_X, gridSize_Y);
//            cell[pos]->setPen(QPen(Qt::red,0));
            cell[pos]->setPen(Qt::NoPen);
//            cell[pos]->setFlag(QGraphicsItem::ItemIsSelectable, true);

            addItem(cell[pos]);

            cell[pos]->setPos(x_pos, y_pos);
            if (pos < 10)
            {
                printf("%d at %f,%f\n",pos,x_pos,y_pos);
                fflush(stdout);
            }
//            cell[pos]->setFlag(QGraphicsItem::ItemIsSelectable);
            glyph[pos] = new Text(x, y, cell[pos]);
            glyph[pos]->setFlag(QGraphicsItem::ItemIsSelectable);

            uscore[pos] = new QGraphicsLineItem(0, 0, gridSize_X, 0);

            addItem(uscore[pos]);

            uscore[pos]->setZValue(1);
            uscore[pos]->setPos(x_pos, y_pos + gridSize_Y);

//            glyph[pos]->setPos(0,0);
//            glyph[pos]->setScale(1.5);
        }
    }

    clear();
    setFont(QFont("ibm3270", 11));

    cursor = new QGraphicsRectItem(cell[0]);
    cursor->setRect(cell[0]->rect());
    cursor->setPos(cell[0]->boundingRect().left(), cell[0]->boundingRect().top());
    cursor->setBrush(Qt::lightGray);
    cursor->setOpacity(0.5);
    cursor->setPen(Qt::NoPen);

    crosshair_X = new QGraphicsLineItem(0, 0, 0, height());
    crosshair_Y = new QGraphicsLineItem(0, 0, width(), 0);

    crosshair_X->setPen(QPen(Qt::white, 0));
    crosshair_Y->setPen(QPen(Qt::white, 0));

//    crosshair_X->pen().setCosmetic(true);
//    crosshair_Y->pen().setCosmetic(true);

    addItem(crosshair_X);
    addItem(crosshair_Y);

    crosshair_X->hide();
    crosshair_Y->hide();

    ruler = false;
    blinkShow = false;
    cursorShow = true;

    int statusPos = (screen_y * gridSize_Y) + gridSize_Y;

    QGraphicsLineItem *s = new QGraphicsLineItem(0, 0, screen_x * gridSize_X, 0);
    s->setPos(0, statusPos++);
    s->setPen(QPen(QColor(0x80, 0x80, 0xFF), 0));
    addItem(s);

    QFont statusBar = QFont("ibm3270");
    statusBar.setPixelSize(gridSize_Y * .75);

    QGraphicsSimpleTextItem *st = new QGraphicsSimpleTextItem("4-A");
    st->setPos(0, statusPos);
    st->setFont(statusBar);
    st->setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    statusXSystem = new QGraphicsSimpleTextItem("");
    statusXSystem->setPos(gridSize_X * (screen_x * .2), statusPos);
    statusXSystem->setFont(statusBar);
    statusXSystem->setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    statusInsert = new QGraphicsSimpleTextItem("");
    statusInsert->setPos(gridSize_X * (screen_x * .2), statusPos);
    statusInsert->setFont(statusBar);
    statusInsert->setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    statusCursor = new QGraphicsSimpleTextItem("000,000");
    statusCursor->setPos(gridSize_X * (screen_x * .75), statusPos);
    statusCursor->setFont(statusBar);
    statusCursor->setBrush(QBrush(QColor(0x80, 0x80, 0xFF)));

    addItem(st);
    addItem(statusXSystem);
    addItem(statusCursor);
    addItem(statusInsert);
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

    if (fontScaling)
    {
        QFontMetrics *fm = new QFontMetrics(font);
        QRectF boxRect = fm->boundingRect("┼");

        printf("DisplayScreen   : FontMetrics: %d x %d    Box char %f x %f   GridSize: %f x %f\n", fm->averageCharWidth(), fm->height(), boxRect.width(), boxRect.height(), gridSize_X, gridSize_Y);
        fflush(stdout);

        tr.scale(gridSize_X / boxRect.width(), gridSize_Y / boxRect.height());
    }
    else
    {
        tr.scale(1,1);
    }

    for (int i = 0; i < screenPos_max; i++)
    {
        glyph[i]->setFont(QFont(font));
        glyph[i]->setTransform(tr);
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
        cell[i]->setBrush(palette[0]);

        uscore[i]->setVisible(false);

        glyph[i]->setBrush(palette[1]);
        glyph[i]->setText(0x00, 0x00, false);

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
        glyph[pos]->setText(EBCDICtoASCIImap[c], c, false);
    }
    else
    {
        glyph[pos]->setText(EBCDICtoASCIImapge[c], c, true);
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
        glyph[pos]->setBrush(cell[pos]->brush());
    }
    else
    {
        if (attrs[pos].reverse)
        {
            cell[pos]->setBrush(palette[attrs[pos].colNum]);
            glyph[pos]->setBrush(palette[0]);
            printf("<reverse>");
        }
        else
        {
            glyph[pos]->setBrush(palette[attrs[pos].colNum]);
            cell[pos]->setBrush(palette[0]);
        }
    }

    //
    // Underscore processing
    //

    if (attrs[pos].uscore)
    {
        uscore[pos]->setVisible(true);
        uscore[pos]->setPen(QPen(palette[attrs[pos].colNum],0));
 //       uscore[pos]->pen().setCosmetic(true);
 //       uscore[pos]->pen().setColor(palette[attrs[pos].colNum]);

        printf("<uscore %d>", attrs[pos].colNum);
    }
    else
    {
        uscore[pos]->setVisible(false);
    }
}

unsigned char DisplayScreen::getChar(int pos)
{
    return (glyph[pos]->text().toUtf8()[0]);
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
            printf(" ** Not implemented **");
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
            attrs[pos].colNum = 1; /* Blue */
        }
        else if (attrs[pos].prot && attrs[pos].intensify)
        {
            attrs[pos].colNum = 7;  /* White */
        }
        else if (!attrs[pos].prot && !attrs[pos].intensify)
        {
            attrs[pos].colNum = 4;   /* Green */
        }
        else
        {
            attrs[pos].colNum = 2;    /* Red */
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
    attrs[pos].extended = true;
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
    glyph[start]->setText(IBM3270_CHAR_NULL, IBM3270_CHAR_NULL, false);
    uscore[start]->setVisible(false);
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
                cell[offset]->setBrush(palette[attrs[offset].colNum]);
                glyph[offset]->setBrush(palette[0]);
            }
            else
            {
                cell[offset]->setBrush(palette[0]);
                glyph[offset]->setBrush(palette[attrs[offset].colNum]);
            }
            if (attrs[offset].uscore)
            {
                uscore[offset]->setPen(QPen(palette[attrs[offset].colNum],0));
//                uscore[offset]->pen().setCosmetic(true);
//                uscore[offset]->pen().setColor(palette[attrs[offset].colNum]);
            }
            else
            {
                uscore[offset]->setVisible(false);
            }
        }
        else
        {
            cell[offset]->setBrush(palette[0]);
            glyph[offset]->setBrush(palette[0]);
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
//            unsigned char thisChar = glyph[i%(SCREENX*SCREENY)]->text().toUtf8()[0];
//            printf("%4d = %c (%2.2X)\n", i, thisChar, thisChar);
            fflush(stdout);
            int offset = i % screenPos_max;
            if (attrs[offset].prot || attrs[offset].fieldStart)
            {
                break;
            }
            if (glyph[offset]->getEBCDIC() == IBM3270_CHAR_NULL)
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
            geActive = glyph[offsetPrev]->getGraphic();
            setChar(offset, glyph[offsetPrev]->getEBCDIC(), true);
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

    for(int fld = pos; fld < endPos - 1 && glyph[fld % screenPos_max]->getEBCDIC() != IBM3270_CHAR_NULL; fld++)
    {
        int offset = fld % screenPos_max;
        int offsetNext = (fld + 1) % screenPos_max;

        attrs[offset] = attrs[offsetNext];
        bool tmpGE = geActive;
        setChar(offset, glyph[offsetNext]->getEBCDIC(), true);
        geActive = tmpGE;
    }

    glyph[endPos - 1]->setText(IBM3270_CHAR_NULL, IBM3270_CHAR_NULL, false);
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
        glyph[i % screenPos_max]->setText(0x00, 0x00, false);
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
                glyph[i]->setText(" ", IBM3270_CHAR_SPACE, false);
        }
    }
}

void DisplayScreen::setCursor(int pos)
{
    cursor->setParentItem(cell[pos]);
    cursor->setBrush(palette[attrs[pos].colNum]);
    cursor->setPos(cell[pos]->boundingRect().left(), cell[pos]->boundingRect().top());
}

void DisplayScreen::showCursor()
{
    cursor->show();
}

void DisplayScreen::setStatusXSystem(QString text)
{
    statusXSystem->setText(text);
}

void DisplayScreen::showStatusCursorPosition(int x, int y)
{
    statusCursor->setText(QString("%1,%2").arg(x + 1, 3).arg(y + 1, -3));
}

void DisplayScreen::showStatusInsertMode(bool ins)
{
    if (ins)
    {
        statusInsert->setText("^");
    }
    else
    {
        statusInsert->setText("");
    }
}

void DisplayScreen::toggleRuler()
{
    ruler = !ruler;

    if (ruler)
    {
        crosshair_X->show();
        crosshair_Y->show();
    }
    else
    {
        crosshair_X->hide();
        crosshair_Y->hide();
    }
}

void DisplayScreen::drawRuler(int x, int y)
{
    if (ruler)
    {
       crosshair_X->setLine((qreal) x * gridSize_X, 0, (qreal) x * gridSize_X, height());
       crosshair_Y->setLine(0 , (qreal) (y + 1) * gridSize_Y, width(), (qreal) (y + 1) * gridSize_Y);
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
                glyph[i]->setBrush(palette[attrs[i].colNum]);
            }
            else
            {
                glyph[i]->setBrush(palette[0]);
            }
        }
    }
}

void DisplayScreen::cursorBlink()
{
    cursorShow = !cursorShow;

    if (!cursorShow)
    {
        cursor->hide();
    }
    else
    {
        cursor->show();
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

void DisplayScreen::getModifiedFields(Buffer *buffer)
{
    for(int i = 0; i < screenPos_max; i++)
    {
        if (attrs[i].fieldStart && !attrs[i].prot)
        {
            int firstField = i;
            int thisField = i;
            do
            {
                if (attrs[thisField].mdt && !attrs[thisField].prot)
                {
                    buffer->add(IBM3270_SBA);

                    printf("Adding field at %d : ", thisField);

                    int nextPos = (thisField + 1) % screenPos_max;

                    if (nextPos < 4096) // 12 bit
                    {
                        buffer->add(twelveBitBufferAddress[(nextPos>>6)&63]);
                        buffer->add(twelveBitBufferAddress[(nextPos&63)]);
                    }
                    else if (nextPos < 16384) // 14 bit
                    {
                        buffer->add((nextPos>>8)&63);
                        buffer->add(nextPos&0xFF);
                    }
                    else // 16 bit
                    {
                        buffer->add((nextPos>>8)&0xFF);
                        buffer->add(nextPos&0xFF);
                    }


                    attrs[thisField].mdt = false;

                    do
                    {
                        uchar b = glyph[thisField++]->getEBCDIC();
                        if (b != IBM3270_CHAR_NULL)
                        {
                            buffer->add(b);
                            printf("%c", b);
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
        printf("%2.2X ", glyph[i]->getEBCDIC());
    }
    printf("\n---- SCREEN ----\n");
    fflush(stdout);
}

void DisplayScreen::dumpAttrs(int pos)
{
    printf("   Attrs: Prot:%d Ext:%d Start:%d Skip:%d Display:%d Uscore:%d Rev:%d Blnk:%d Intens:%d Num:%d Pen:%d\n",
           attrs[pos].prot, attrs[pos].extended, attrs[pos].fieldStart, attrs[pos].askip, attrs[pos].display, attrs[pos].uscore, attrs[pos].reverse, attrs[pos].blink, attrs[pos].intensify, attrs[pos].num, attrs[pos].pen);
}
