#include "DisplayData.h"

DisplayData::DisplayData(QGraphicsScene *parent, int screen_x, int screen_y)
{

    DisplayView *d = (DisplayView *)parent->views().first();

    screen = new QGraphicsScene(0, 0, parent->width(), parent->height());

    this->screen_x = screen_x;
    this->screen_y = screen_y;

    gridSize_X = ((qreal) d->width()) / (qreal) screen_x;
    gridSize_Y = ((qreal) d->height()) / (qreal) screen_y;

    screenPos_max = screen_x * screen_y;

    printf("Screen size: %d x %d - gridsize: %lf x %lf\n", d->width(), d->height(), gridSize_X, gridSize_Y);
    fflush(stdout);

    QPen p;
    p.setCosmetic(false);
    p.setWidth(0);

    QPen u;
    u.setWidth(8);
    u.setBrush(Qt::green);

    attrs = new Attributes[screenPos_max];
    glyph = new Text*[screenPos_max];
    cell = new QGraphicsRectItem*[screenPos_max];
    uscore = new QGraphicsLineItem*[screenPos_max];

    for(int y = 0; y < screen_y; y++)
    {
        qreal y_pos = y * gridSize_Y;

        for(int x = 0; x < screen_x; x++)
        {
            int pos = x + (y * screen_x);

            qreal x_pos = x * gridSize_X;

            cell[pos] = new QGraphicsRectItem(x_pos/2, y_pos/2, gridSize_X, gridSize_Y);
            uscore[pos] = new QGraphicsLineItem(cell[pos]->boundingRect().left() + 1, cell[pos]->boundingRect().bottom() - 1, cell[pos]->boundingRect().right(), cell[pos]->boundingRect().bottom() -1, cell[pos]);

            cell[pos]->setPos(x_pos/2, y_pos/2);

            glyph[pos] = new Text(cell[pos]);

            screen->addItem(cell[pos]);
        }
    }

    clear();
    setFont(QFont("Hack", 11));

    cursor = new QGraphicsRectItem(cell[0]);
    cursor->setRect(cell[0]->rect());
    cursor->setPos(cell[0]->boundingRect().left(), cell[0]->boundingRect().top());
    cursor->setBrush(Qt::lightGray);
    cursor->setOpacity(0.5);

    crosshair_X = new QGraphicsLineItem(0, 0, 0, screen->height());
    crosshair_Y = new QGraphicsLineItem(0, 0, screen->width(), 0);

    crosshair_X->setPen(QPen(Qt::white));
    crosshair_Y->setPen(QPen(Qt::white));

    screen->addItem(crosshair_X);
    screen->addItem(crosshair_Y);

    crosshair_X->hide();
    crosshair_Y->hide();

    ruler = false;
}

int DisplayData::width()
{
    return screen_x;
}

int DisplayData::height()
{
    return screen_y;
}

int DisplayData::gridWidth()
{
    return gridSize_X;
}

int DisplayData::gridHeight()
{
    return gridSize_Y;
}

void DisplayData::setParent(QGraphicsScene *scene)
{
    screen->setParent(scene);
}

QGraphicsScene *DisplayData::getScene()
{
    return screen;
}

void DisplayData::setFont(QFont font)
{
    for (int i = 0; i < screenPos_max; i++)
    {
        glyph[i]->setFont(QFont(font));
    }
}

void DisplayData::clear()
{
    for(int i = 0; i < screenPos_max; i++)
    {
        cell[i]->setBrush(palette[0]);
        uscore[i]->setPen(uscore_pen[0]);
        uscore[i]->setVisible(false);

        glyph[i]->setBrush(palette[1]);
        glyph[i]->setText(0x00);

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
}

void DisplayData::setChar(int pos, unsigned char c, bool move)
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

    glyph[pos]->setText(QString(EBCDICtoASCIImap[c]));

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
        uscore[pos]->setPen(QPen(palette[attrs[pos].colNum], 2));

        printf("<uscore>");
    }
    else
    {
        uscore[pos]->setVisible(false);
    }

    if (c != IBM3270_CHAR_NULL)
    {
        printf("%c", EBCDICtoASCIImap[c]);
    }
    else
    {
        printf("0x00");
    }

    fflush(stdout);
}

unsigned char DisplayData::getChar(int pos)
{
    return (glyph[pos]->text().toUtf8()[0]);
}


void DisplayData::setCharAttr(unsigned char extendedType, unsigned char extendedValue)
{
/*    if (!useCharAttr)
    {
        charAttr.blink = attrs[pos].blink;
        charAttr.colour = attrs[pos].colour;
        charAttr.uscore = attrs[pos].uscore;
        charAttr.reverse = attrs[pos].reverse;

    } */
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
//            extAttr.reverse = false;
            printf("fg colour %s", colName[charAttr.colNum]);
            break;
        case IBM3270_EXT_BG_COLOUR:
            charAttr.colour = palette[extendedValue&7];
            charAttr.colNum = extendedValue&7;
            charAttr.colour_default = false;
            printf("bg colour %s", colName[charAttr.colNum]);
            //            extAttr.reverse = true;
            break;
        default:
            printf(" ** Not implemented **");
    }
    printf("]");
    fflush(stdout);

    useCharAttr = true;

}

void DisplayData::resetCharAttr()
{
    charAttr.blink_default = true;
    charAttr.reverse_default = true;
    charAttr.uscore_default = true;
    charAttr.colour_default = true;
}

void DisplayData::setField(int pos, unsigned char c, bool sfe)
{
    printf("3270 Attribute %2.2X at %d", c, pos);

    attrs[pos].prot = (c>>5)&1;
    attrs[pos].num  = (c>>4)&1;
    attrs[pos].display = (((c>>2)&3) != 3);
    attrs[pos].pen = (( c >> 2) & 3) == 2 || (( c >> 2) & 3) == 1;
    attrs[pos].intensify = ((c >> 2) & 3) == 2;
    attrs[pos].mdt = c & 1;
    attrs[pos].extended = sfe;
/*
    if (!useCharAttr)
    {
        attrs[pos].charAttr = false;
    }
    else
    {
        attrs[pos].charAttr = true;
    }
*/
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
        //uscore[pos]->hide();
        //attrs[pos].uscore = false;
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

//    int py = pos / SCREENX;
//    int px = pos - (py * SCREENX);

    if (!sfe)
    {
        setFieldAttrs(pos);
    }

//    printf("[S%4d - %2d,%2d): Attribute byte: %2.2X prot=%d num=%d display=%d pen=%d mdt=%d reverse=%d intens=%d : ", pos, px, py, c, attrs[pos].prot, attrs[pos].num, attrs[pos].display, attrs[pos].pen, attrs[pos].mdt, attrs[pos].reverse, attrs[pos].intensify);

}

void DisplayData::resetExtended(int pos)
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


void DisplayData::resetExtendedHilite(int pos)
{
    attrs[pos].uscore = false;
    attrs[pos].blink = false;
    attrs[pos].reverse = false;
}

void DisplayData::setExtendedColour(int pos, bool foreground, unsigned char c)
{
    attrs[pos].colNum = c&7;
    attrs[pos].reverse = !foreground;
    attrs[pos].extended = true;
    if(foreground)
    {
        printf(" %s]", colName[attrs[pos].colNum]);
    }
}

void DisplayData::setExtendedBlink(int pos)
{
    attrs[pos].reverse = false;
    attrs[pos].blink = true;
    printf("[Blink]");
}

void DisplayData::setExtendedReverse(int pos)
{
    attrs[pos].blink = false;
    attrs[pos].reverse = true;
    printf("[Reverse]");
}

void DisplayData::setExtendedUscore(int pos)
{
    attrs[pos].uscore = true;
    printf("[UScore]");
}

void DisplayData::setFieldAttrs(int start)
{
    attrs[start].fieldStart = true;

    printf("[setting field %d to uscore %d colour %s]", start, attrs[start].uscore, colName[attrs[start].colNum]);
    fflush(stdout);

    resetFieldAttrs(start);

    glyph[start]->setText(IBM3270_CHAR_NULL);
    uscore[start]->setVisible(false);
}


int DisplayData::resetFieldAttrs(int start)
{
    int lastField = findField(start);

    printf("[attributes obtained from %d]", lastField);

    int endPos = start + screenPos_max;

    for(int i = start; i < endPos; i++)
    {
        int offset = i % screenPos_max;
//        bool uscore = attrs[offset].uscore;

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

//        if (!useCharAttr)
  //      {
            attrs[offset].colNum = attrs[lastField].colNum;
            attrs[offset].uscore = attrs[lastField].uscore;
            attrs[offset].blink = attrs[lastField].blink;
            attrs[offset].reverse = attrs[lastField].reverse;
            attrs[offset].charAttr = false;
    //    }

        //        attrs[offset] = attrs[start];
//        attrs[offset].fieldStart = false;

        if (attrs[offset].display)
        {
//            if (!attrs[offset].charAttr)
//            {
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
                    uscore[offset]->setPen(QPen(palette[attrs[offset].colNum], 2));
                    uscore[offset]->setVisible(true);
                }
                else
                {
                  uscore[offset]->setVisible(false);
                }
  //          }
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


bool DisplayData::insertChar(int pos, unsigned char c, bool insertMode)
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
            if (glyph[offset]->text()[0] == IBM3270_CHAR_NULL)
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
        for(int fld = endPos; fld > pos; fld--)
        {
            int offset = fld % screenPos_max;
            int offsetPrev = (fld - 1) % screenPos_max;

            printf("Moving %c to %d\n", glyph[offsetPrev]->toUChar(), offset);
            fflush(stdout);
            attrs[offset] = attrs[offsetPrev];
            setChar(offset, ASCIItoEBCDICmap[glyph[offsetPrev]->toUChar()], true);
        }
    }

    printf("MDT set for %d\n", thisField);
    attrs[thisField].mdt = true;
    setChar(pos, ASCIItoEBCDICmap[c], false);

    return true;
}

void DisplayData::deleteChar(int pos)
{
    if (attrs[pos].prot)
    {
        printf("Protected!\n");
        fflush(stdout);
        return;
    }

    int nextField = findNextField(pos);
    int endPos = nextField;

    if (nextField < pos)
    {
        nextField += screenPos_max;
    }

    for(int fld = pos; fld < endPos - 1 && glyph[fld % screenPos_max]->text() != IBM3270_CHAR_NULL; fld++)
    {
        int offset = fld % screenPos_max;
        int offsetNext = (fld + 1) % screenPos_max;

        attrs[offset] = attrs[offsetNext];
        setChar(offset, ASCIItoEBCDICmap[glyph[offsetNext]->toUChar()], true);
    }

    glyph[endPos - 1]->setText(IBM3270_CHAR_NULL);
    attrs[findField(pos)].mdt = true;
}

void DisplayData::eraseEOF(int pos)
{
    int nextField = findNextField(pos);

    if (nextField < pos)
    {
        nextField += screenPos_max;
    }

    /* Blank field */
    for(int i = pos; i < nextField; i++)
    {
        glyph[i % screenPos_max]->setText(0x00);
    }

    attrs[findField(pos)].mdt = true;
}

void DisplayData::eraseUnprotected(int start, int end)
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
                glyph[i]->setText(" ");
        }
    }
}

void DisplayData::setCursor(int pos)
{
    cursor->setParentItem(cell[pos]);
    cursor->setPos(cell[pos]->boundingRect().left(), cell[pos]->boundingRect().top());
}

void DisplayData::toggleRuler()
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

void DisplayData::drawRuler(int x, int y)
{
    if (ruler)
    {
       crosshair_X->setLine(x * gridSize_X, 0, x * gridSize_X, screen_y * gridSize_Y);
       crosshair_Y->setLine(0 , (y + 1) * gridSize_Y - 1, screen_x * gridSize_X, (y + 1) * gridSize_Y - 1);
    }
}

int DisplayData::findField(int pos)
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

int DisplayData::findNextField(int pos)
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

int DisplayData::findNextUnprotectedField(int pos)
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

int DisplayData::findPrevUnprotectedField(int pos)
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
        // Check this position for unprotected and fieldStart and check the position for
        // fieldStart - an unprotected field cannot start where two fieldStarts are adajacent
        tmpNxt = (tmpPos + 1) % screenPos_max;
        if (attrs[tmpPos].fieldStart && !attrs[tmpPos].prot && !attrs[tmpNxt].fieldStart)
        {
            return tmpPos;
        }
    }
    printf("No unprotected field found: start = %d, end = %d\n", pos, pos + screenPos_max);
    fflush(stdout);
    return 0;
}

void DisplayData::getModifiedFields(Buffer *buffer)
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
                        uchar b = glyph[thisField++]->toUChar();
                        if (b != IBM3270_CHAR_NULL)
                        {
                            buffer->add(ASCIItoEBCDICmap[b]);
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

void DisplayData::dumpFields()
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


void DisplayData::dumpDisplay()
{
    printf("---- SCREEN ----");
    for (int i = 0; i < screenPos_max; i++)
    {
        if (i % screen_x == 0)
        {
            printf("\n");
        }
        printf("%2.2X ", glyph[i]->toUChar());
    }
    printf("\n---- SCREEN ----\n");
    fflush(stdout);
}

void DisplayData::dumpAttrs(int pos)
{
    printf("   Attrs: Prot:%d Ext:%d Start:%d Skip:%d Display:%d Uscore:%d Rev:%d Blnk:%d Intens:%d Num:%d Pen:%d\n",
           attrs[pos].prot, attrs[pos].extended, attrs[pos].fieldStart, attrs[pos].askip, attrs[pos].display, attrs[pos].uscore, attrs[pos].reverse, attrs[pos].blink, attrs[pos].intensify, attrs[pos].num, attrs[pos].pen);
}
