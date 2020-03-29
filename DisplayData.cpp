#include "DisplayData.h"


#define SCREENX 80
#define SCREENY 24

DisplayData::DisplayData(QGraphicsScene *screen, int size)
{

    DisplayView *d = (DisplayView *)screen->views().first();

    gridSize_X = (d->width()) / 80;
    gridSize_Y = (d->height()) / 24;

    printf("Screen size: %d x %d - gridsize: %d x %d\n", d->width(), d->height(), gridSize_X, gridSize_Y);
    fflush(stdout);

    QPen p;
    p.setCosmetic(false);
    p.setWidth(0);

    attrs = new Attributes[size];
    glyph = new Text*[size];
    cell = new QGraphicsRectItem*[size];

    this->size = size;

    screen->setBackgroundBrush(Qt::blue);

    for(int y = 0; y < SCREENY; y++)
    {
        int y_pos = y * gridSize_Y;

        for(int x = 0; x < SCREENX; x++)
        {
            int pos = x + (y * SCREENX);

            int x_pos = x * gridSize_X;

            cell[pos] = new QGraphicsRectItem(x_pos/2, y_pos/2, gridSize_X, gridSize_Y);
            cell[pos]->setBrush(Qt::black);
            cell[pos]->setPen(p);
            cell[pos]->setPos(x_pos/2, y_pos/2);

            glyph[pos] = new Text(cell[pos]);
            glyph[pos]->setBrush(palette[4]);
            glyph[pos]->setFont(QFont("Hack", 11));
            glyph[pos]->setText(0x00);

            attrs[pos].prot = false;
            attrs[pos].askip = false;
            attrs[pos].colour = palette[4];
            attrs[pos].reverse = false;
            attrs[pos].uscore = false;
            attrs[pos].blink = false;
            attrs[pos].mdt = false;
            attrs[pos].display = true;
            attrs[pos].extended = false;
            attrs[pos].fieldStart = false;

            screen->addItem(cell[pos]);
        }
    }

    cursor = new QGraphicsRectItem(cell[0]);
    cursor->setRect(cell[0]->rect());
    cursor->setPos(cell[0]->boundingRect().left(), cell[0]->boundingRect().top());
    cursor->setBrush(Qt::lightGray);
    cursor->setOpacity(0.5);
}

void DisplayData::clear()
{
    for(int i = 0; i < SCREENX * SCREENY; i++)
    {
        cell[i]->setBrush(palette[0]);

        glyph[i]->setBrush(palette[1]);
        glyph[i]->setText(0x00);

        attrs[i].askip = false;
        attrs[1].pen = false;
        attrs[i].intensify = false;
        attrs[i].prot = false;
        attrs[i].askip = false;
        attrs[i].colour = palette[1];
        attrs[i].reverse = false;
        attrs[i].uscore = false;
        attrs[i].blink = false;
        attrs[i].mdt = false;
        attrs[i].display = true;
        attrs[i].extended = false;
        attrs[i].fieldStart = false;
    }
}

void DisplayData::setChar(int pos, unsigned char c)
{
    attrs[pos].fieldStart = false;
    glyph[pos]->setText(QString(EBCDICtoASCIImap[c]));
    printf("%c", EBCDICtoASCIImap[c]);
    int thisField = findField(pos);

    attrs[pos] = attrs[thisField];
    attrs[pos].fieldStart = false;
    if (useCharAttr)
    {
        if(charAttr.reverse)
        {
            glyph[pos]->setBrush(palette[0]);
            cell[pos]->setBrush(charAttr.colour);
        }
        else
        {
            glyph[pos]->setBrush(charAttr.colour);
            cell[pos]->setBrush(palette[0]);
        }
        return;
    }
    if (attrs[pos].reverse)
    {
        glyph[pos]->setBrush(palette[0]);
        cell[pos]->setBrush(attrs[pos].colour);
    }
    else
    {
        glyph[pos]->setBrush(attrs[pos].colour);
        cell[pos]->setBrush(palette[0]);
    }
}

unsigned char DisplayData::getChar(int pos)
{
    return (glyph[pos]->text().toUtf8()[0]);
}

void DisplayData::setExtendedColour(int pos, bool foreground, unsigned char c)
{
/*    int nextField = findNextField(pos);
    if (nextField < pos)
    {
        nextField = nextField + SCREENX * SCREENY;
    }
*/
    attrs[pos].colour = palette[c&7];
    attrs[pos].reverse = !foreground;
    attrs[pos].fieldStart = true;
    attrs[pos].extended = true;
/*
    for(int i = pos; i < nextField; i++)
    {
        int offset = i%(SCREENX * SCREENY);
        attrs[offset] = attrs[pos];
        if (attrs[offset].display)
        {
            if (attrs[offset].reverse)
            {
                cell[offset]->setBrush(attrs[offset].colour);
                glyph[offset]->setBrush(palette[0]);
            }
            else
            {
                cell[offset]->setBrush(palette[0]);
                glyph[offset]->setBrush(attrs[offset].colour);
            }
        }
        else
        {
            cell[offset]->setBrush(palette[0]);
            glyph[offset]->setBrush(palette[0]);
        }
    }
*/
}

void DisplayData::setCharAttr(int pos, unsigned char extendedType, unsigned char extendedValue)
{
    printf("SetAttribute(%d, %d) - %2.2X : %2.2X\n", pos - (int)((pos / SCREENX) * SCREENX), (int)pos/SCREENX, extendedType, extendedValue);
    fflush(stdout);
    switch(extendedType)
    {
        case IBM3270_EXT_DEFAULT:
            charAttr.blink = false;
            charAttr.reverse = false;
            charAttr.uscore = false;
            charAttr.colour = palette[1];
            return;
        case IBM3270_EXT_HILITE:
            switch(extendedValue)
            {
                case IBM3270_EXT_HI_NORMAL:
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink   = false;
                    break;
                case IBM3270_EXT_HI_BLINK:
                    charAttr.blink   = true;
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    break;
                case IBM3270_EXT_HI_REVERSE:
                    charAttr.blink   = false;
                    charAttr.uscore  = false;
                    charAttr.reverse = true;
                    break;
                case IBM3270_EXT_HI_USCORE:
                    charAttr.blink   = false;
                    charAttr.uscore  = true;
                    charAttr.reverse = false;
                    break;
                default:
                    charAttr.uscore  = false;
                    charAttr.reverse = false;
                    charAttr.blink   = false;
                    break;
            }
            break;
        case IBM3270_EXT_FG_COLOUR:
            charAttr.colour = palette[extendedValue&7];
//            extAttr.reverse = false;
            break;
        case IBM3270_EXT_BG_COLOUR:
            charAttr.colour = palette[extendedValue&7];
//            extAttr.reverse = true;
            break;
        default:
            printf("Not implemented: SA order %2.2X : %2.2X\n", extendedType, extendedValue);
            fflush(stdout);
    }

    useCharAttr = true;

}

void DisplayData::resetCharAttr()
{
    useCharAttr = false;
}

void DisplayData::setField(int pos, unsigned char c)
{
    int nextField = findNextField(pos);
    if (nextField < pos)
    {
        nextField = nextField + SCREENX * SCREENY;
    }
    printf("Attribute %2.2X", c);

    decodeFieldAttribute(c, attrs[pos]);
    //TODO: Attribute conflict resolution

    for(int i = pos; i < nextField; i++)
    {
        int offset = i%(SCREENX * SCREENY);
        attrs[offset] = attrs[pos];
        attrs[offset].fieldStart = false;
        if (attrs[offset].display)
        {
            if (attrs[offset].reverse)
            {
                cell[offset]->setBrush(attrs[offset].colour);
                glyph[offset]->setBrush(palette[0]);
            }
            else
            {
                cell[offset]->setBrush(palette[0]);
                glyph[offset]->setBrush(attrs[offset].colour);
            }
        }
        else
        {
            cell[offset]->setBrush(palette[0]);
            glyph[offset]->setBrush(palette[0]);
        }
    }
    attrs[pos].fieldStart = true;
    glyph[pos]->setText(IBM3270_CHAR_NULL);

    int py = pos / SCREENX;
    int px = pos - (py * SCREENX);

//    printf("[S%4d - %2d,%2d): Attribute byte: %2.2X prot=%d num=%d display=%d pen=%d mdt=%d reverse=%d intens=%d : ", pos, px, py, c, attrs[pos].prot, attrs[pos].num, attrs[pos].display, attrs[pos].pen, attrs[pos].mdt, attrs[pos].reverse, attrs[pos].intensify);

}

void DisplayData::decodeFieldAttribute(unsigned char attr, Attributes &f)
{
    //TODO: extended attributes with 3270 field attributes

    f.prot = (attr>>5)&1;
    f.num  = (attr>>4)&1;
    f.display = (((attr>>2)&3) != 3);
    f.pen = ((attr>>2)&3) == 2 || ((attr>>2)&3) == 1;
    f.intensify = ((attr>>2)&3) == 2;
    f.mdt = (attr)&1;

    f.askip = (f.prot & f.num);

//    printf("P:%d N:%d D:%d L:%d I:%d M:%d A:%d\n", f.prot, f.num, f.display, f.pen, f.intensify, f.mdt, f.askip);

    if (!f.extended)
    {
        if (f.prot && !f.intensify)
        {
            f.colour = palette[1]; /* Blue */
        }
        else if (f.prot && f.intensify)
        {
            f.colour = palette[7]; //TODO: use names  (white)
        }
        else if (!f.prot && !f.intensify)
        {
            f.colour = palette[4]; /* Green */
        }
        else
        {
            f.colour = palette[2]; /* Red */
        }
    }
    if(!f.prot)
    {
        printf("(unprot,");
    }
    else
    {
        printf("(prot,");
    }
    if(f.intensify)
    {
        printf("intens,");
    }
    if (f.askip)
    {
        printf("askip,");
    }
    if (!f.display)
    {
        printf("nondisp,");
    }
    if (f.pen)
    {
        printf("pen,");
    }
    if (f.num)
    {
        printf("num,");
    }
    if (f.mdt)
    {
        printf("mdt,");
    }
    printf(")");
    fflush(stdout);
}

bool DisplayData::insertChar(int pos, unsigned char c, bool insertMode)
{
    if (attrs[pos].prot)
    {
        printf("Protected!\n");
        fflush(stdout);
        return false;
    }

    if (insertMode)
    {
        int endPos = -1;
        for(int i = pos; i < (pos + (SCREENX * SCREENY)); i++)
        {
            unsigned char thisChar = glyph[i%(SCREENX*SCREENY)]->text().toUtf8()[0];
//            printf("%4d = %c (%2.2X)\n", i, thisChar, thisChar);
            fflush(stdout);
            if (glyph[i%(SCREENX * SCREENY)]->text()[0] == IBM3270_CHAR_NULL)
            {
                endPos = i;
                break;
            }
            if (attrs[i%(SCREENX * SCREENY)].prot)
            {
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
            glyph[fld%(SCREENX * SCREENY)]->setText(glyph[+((fld-1)%(SCREENX * SCREENY))]->text());
        }
    }

    int thisField = findField(pos);
    printf("MDT set for %d\n", thisField);
    attrs[thisField].mdt = true;
    if (!attrs[thisField].display)
    {
        glyph[pos]->setBrush(cell[pos]->brush());
    }
    glyph[pos]->setText(QString(c));

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
        nextField = nextField + (SCREENX * SCREENY);
    }

    for(int fld = pos; fld < endPos - 1 && glyph[pos]->text() != IBM3270_CHAR_NULL; fld++)
    {
        glyph[fld%(SCREENX*SCREENY)]->setText(glyph[(fld+1)%(SCREENX*SCREENY)]->text());
    }

    glyph[endPos]->setText(IBM3270_CHAR_NULL);

    attrs[findField(pos)].mdt = true;

}

void DisplayData::eraseEOF(int pos)
{
    int nextField = findNextField(pos);

    if (nextField < pos)
    {
        nextField+= SCREENX * SCREENY;
    }

    /* Blank field */
    for(int i = pos; i < nextField; i++)
    {
        glyph[i%(SCREENX * SCREENY)]->setText(0x00);
    }

    attrs[findField(pos)].mdt = true;
}

void DisplayData::setCursor(int pos)
{
    cursor->setParentItem(cell[pos]);
    cursor->setPos(cell[pos]->boundingRect().left(), cell[pos]->boundingRect().top());
}

int DisplayData::findField(int pos)
{
    int endPos = pos - (SCREENX * SCREENY);
//    printf("findField: endpos = %d\n", endPos);
//    fflush(stdout);
    for (int i = pos; i > endPos ; i--)
    {
        int offset = i;
        if (i < 0)
        {
            offset = (SCREENX*SCREENY) + i;
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
    for(int i = pos; i < (pos+(SCREENX * SCREENY)); i++)
    {
        tmpPos = i%(SCREENX * SCREENY);
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
    int tmpPos;
    for(int i = pos; i < (pos+(SCREENX * SCREENY)); i++)
    {
        tmpPos = i%(SCREENX * SCREENY);
        if (attrs[tmpPos].fieldStart & !attrs[tmpPos].prot)
        {
            return tmpPos;
        }
    }
    printf("No unprotected field found: start = %d, end = %d\n", pos, pos +(SCREENX * SCREENY));
    fflush(stdout);
    return 0;
}

void DisplayData::getModifiedFields(Buffer *buffer)
{
    for(int i = 0; i < (SCREENX * SCREENY); i++)
    {
        if (attrs[i].fieldStart & !attrs[i].prot)
        {
            int firstField = i;
            int thisField = i;
            do
            {
                if (attrs[thisField].mdt & !attrs[thisField].prot)
                {
                    buffer->add(IBM3270_SBA);

                    printf("Adding field at %d : ", thisField);

                    int nextPos = (thisField + 1)%(SCREENX * SCREENY);

                    buffer->add(twelveBitBufferAddress[(nextPos>>6)&63]);
                    buffer->add(twelveBitBufferAddress[(nextPos&63)]);

                    attrs[thisField].mdt = false;

                    do
                    {
                        uchar b = glyph[thisField++]->toUChar();
                        if (b != IBM3270_CHAR_NULL)
                        {
                            buffer->add(ASCIItoEBCDICmap[b]);
                            printf("%c", b);
                        }
                        thisField = thisField % (SCREENX * SCREENY);
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
    for(int i = 0; i < (SCREENX * SCREENY); i++)
    {
        if (attrs[i].fieldStart)
        {
            int tmpy = i / SCREENX;
            int tmpx = i - (tmpy * SCREENX);

            printf("Field at %4d (%2d,%2d) : Prot: %d\n", i, tmpx, tmpy, attrs[i].prot);
        }
    }
    fflush(stdout);
}


void DisplayData::dumpDisplay()
{
    printf("---- SCREEN ----");
    for (int i = 0; i < (SCREENX * SCREENY); i++)
    {
        if (i%SCREENX == 0)
        {
            printf("\n");
        }
        printf("%2.2X ", glyph[i]->toUChar());
    }
    printf("\n---- SCREEN ----\n");
    fflush(stdout);
}
