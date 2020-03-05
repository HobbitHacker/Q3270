/*
 * Copyright 2020 Andy Styles <andy@styles.homeip.net>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DisplayDataStream.h"

#include <stdlib.h>
#include <QObject>
#include <QDebug>
#include <chrono>
#include <thread>

DisplayDataStream::DisplayDataStream(QGraphicsScene* parent)
{
    display = parent;

    //TODO: screen sizes
    defaultScreenSize = 80*24;
    alternateScreenSize = 80*24;

    primary_x = 0;
    primary_y = 0;

    cursor_x = 0;
    cursor_y = 0;

    count = 0;

    chars = new DisplayData(defaultScreenSize);

    display->setBackgroundBrush(Qt::blue);

    for(int y = 0; y < SCREENY; y++)
    {
        int y_pos = y * GRIDSIZE_Y;

        for(int x = 0; x < SCREENX; x++)
        {

            int pos = x + (y * SCREENX);

            int x_pos = x * GRIDSIZE_X;

            cells[pos] = new QGraphicsRectItem(x_pos, y_pos, GRIDSIZE_X*2, GRIDSIZE_Y*2);
            cells[pos]->setBrush(Qt::black);
            cells[pos]->setPos(x_pos, y_pos);

            glyph[pos] = new Text(cells[pos]);
            glyph[pos]->setBrush(Qt::blue);
            glyph[pos]->setFont(QFont("Consolas", 24));
            glyph[pos]->setText(0x00);

            display->addItem(cells[pos]);
        }
    }

    cursor = new QGraphicsRectItem(cells[0]);
    cursor->setRect(cells[0]->rect());
    cursor->setPos(cells[cursor_x + (cursor_y * SCREENX)]->boundingRect().left(), cells[cursor_x + (cursor_y * SCREENX)]->boundingRect().top());
    cursor->setBrush(Qt::lightGray);
    cursor->setOpacity(0.5);

    chars->clear();

}

void DisplayDataStream::processStream(Buffer *b)
{
    //FIXME: buffer size 0 shouldn't happen!
    if (b->size() == 0)
    {
        return;
    }

    b->dump();

    // Process WRITE command
    extended.on = false;

    switch(b->getByte())
    {
        case IBM3270_EW:
            processEW(b);
            break;
        case IBM3270_W:
            processW(b);
            break;
        case IBM3270_WSF:
            processWSF(b->nextByte());
            break;
        case IBM3270_EWA:
            processEWA(b);
            break;
        default:
            printf("Unrecognised WRITE command: %02.2X\n", b->getByte());
            b->dump();
            processing = false;
            return;
    }

    while(b->nextByte())
    {
//        printf("Processing %02.2X\n", b->getByte());
        switch(b->getByte())
        {
            case IBM3270_SF:
                processSF(b);
                break;
            case IBM3270_SBA:
                processSBA(b);
                break;
            case IBM3270_SFE:
                processSFE(b);
                break;
            case IBM3270_IC:
                processIC();
                break;
            case IBM3270_RA:
                processRA(b);
                break;
            case IBM3270_SA:
                processSA(b);
                break;
            default:
                placeChar(b->getByte());
        }
    }

    b->reset();
    b->setProcessing(false);

    if (resetMDT)
    {

    }

    showFields();
    fflush(stdout);

}

void DisplayDataStream::processWCC(Buffer *b)
{
    uchar wcc = b->getByte();

    int reset = wcc>6&1;

    resetMDT = wcc&1;
    resetKB  = (wcc>>1)&1;
    alarm    = (wcc>>2)&1;

    printf("Seen WCC %02.2X - reset=%d, resetMDT=%d, resetKB=%d, alarm=%d\n", (uchar) wcc, reset, resetMDT, resetKB, alarm);

}

void DisplayDataStream::processEW(Buffer *buf)
{
    processWCC(buf->nextByte());

    primary_x = 0;
    primary_y = 0;

    cursor_x = 0;
    cursor_y = 0;

//    display->setBackgroundBrush(Qt::black);

    for(int i = 0; i < SCREENX * SCREENY; i++)
    {
        cells[i]->setBrush(palette[0]);
        glyph[i]->setBrush(palette[4]);
        glyph[i]->setText(0x00);
    }

    screenFields.clear();
    chars->clear();

    printf("Erase Write\n");
}

void DisplayDataStream::processW(Buffer *buf)
{
    printf("Write\n");

    processWCC(buf->nextByte());
}

void DisplayDataStream::processSF(Buffer *buf)
{
    printf("Start Field\n");

    buf->nextByte();

    int pos = primary_x + (primary_y * SCREENX);

    setAttributes(buf);

    placeChar(0x40);

    screenFields.emplace(pos, FieldFlags{ askip, prot, mdt });

    chars->setChar(pos, buf->getByte());
}

void DisplayDataStream::processSBA(Buffer *buf)
{
    int pos = extractBufferAddress(buf->nextByte());

    primary_y = (pos / SCREENX);
    primary_x = pos - (primary_y * SCREENX);
}

void DisplayDataStream::processSFE(Buffer *b)
{
    printf("SFE seen\n");

    b->nextByte();
}

void DisplayDataStream::processIC()
{
    cursor_x = primary_x;
    cursor_y = primary_y;
    addCursor();
}

void DisplayDataStream::processRA(Buffer *b)
{
    int endPos = extractBufferAddress(b->nextByte());

    if (endPos > (SCREENX * SCREENY) - 1)
    {
        endPos = (SCREENX * SCREENY) - 1;
    }

    int curPos = primary_x + (primary_y * SCREENX);

    b->nextByte();
    uchar newChar = b->getByte();

    if(endPos == curPos)
    {
        int tmp_x = primary_x;
        int tmp_y = primary_y;

        for(int i = 0; i < SCREENX * SCREENY; i++)
        {
            placeChar(newChar);
        }
        primary_x = tmp_x;
        primary_y = tmp_y;
        placeChar(newChar);
    }

    if (endPos > curPos)
    {
        for(int i = curPos; i < endPos; i++)
        {
            placeChar(newChar);
        }
    }
    if (endPos < curPos)
    {
        for(int i = 0; i <= endPos; i++)
        {
            placeChar(newChar);
        }
    }

}

void DisplayDataStream::processSA(Buffer *b)
{
    int extendedType = b->nextByte()->getByte();
    int extendedValue = b->nextByte()->getByte();

    printf("SetAttribute: %02.2X : %02.2X\n", extendedType, extendedValue);
    fflush(stdout);

    switch(extendedType)
    {
        case IBM3270_EXT_DEFAULT:
            extended.on = false;
            extended.highlight = 0;
            extended.foreground = 1;
            extended.background = 0;
            return;
        case IBM3270_EXT_HILITE:
            extended.highlight = extendedValue;
            break;
        case IBM3270_EXT_FG_COLOUR:
            extended.foreground = extendedValue&7;
            break;
        case IBM3270_EXT_BG_COLOUR:
            extended.background = extendedValue&7;
            break;
        default:
            printf("Not implemented: SA order %02.2X : %02.2X\n", extendedType, extendedValue);
            fflush(stdout);
    }

    extended.on = true;
}

void DisplayDataStream::processEWA(Buffer *b)
{
    processEW(b);
}

void DisplayDataStream::processWSF(Buffer *b)
{
    wsfLen = b->getByte()<<8;
    b->nextByte();
    wsfLen += b->getByte();
    b->nextByte();

    printf("WSF - length: %03d; command = %02.2X\n", wsfLen, b->getByte());
    fflush(stdout);

    switch(b->getByte())
    {
        case IBM3270_WSF_RESET:
            WSFreset(b);
            break;
        case IBM3270_WSF_READPARTITION:
            WSFreadPartition(b);
            break;
        default:
            printf("Unimplemented WSF command: %02.2X\n", b->getByte());
            break;
    }
}

void DisplayDataStream::WSFreset(Buffer *b)
{
    printf("Reset Partition %02.2X\n", b->nextByte()->getByte());
    fflush(stdout);
    return;
}

void DisplayDataStream::WSFreadPartition(Buffer *b)
{
    uchar partition = b->nextByte()->getByte();
    uchar type = b->nextByte()->getByte();

    printf("ReadPartition %d - type %02.2X\n", partition, type);

    Buffer *queryReply = new Buffer();
    queryReply->add(IBM3270_WSF_QUERYREPLY);

    replySummary(queryReply);

    emit(bufferReady(queryReply));

}

void DisplayDataStream::replySummary(Buffer *buffer)
{
    unsigned char qrt[] = { 0x80, 0x86, 0x87, 0xA6 };

    buffer->add(strlen((char*)qrt)>>8);
    buffer->add(strlen((char*)qrt)&0xFF);

    for(int i = 0; i < strlen((char*)qrt); i++)
    {
        buffer->add(qrt[i]);
    }
}

int DisplayDataStream::extractBufferAddress(Buffer *b)
{
    //TODO: non-12/14 bit addresses & EBCDIC characters
    int sba1 = b->getByte();
    int sba2 = b->nextByte()->getByte();

    sba1 = sba1&63;
    sba2 = sba2&63;

    return (sba1<<6)|sba2;
}

void DisplayDataStream::setAttributes(Buffer *b)
{

    //TODO: extended attributes with 3270 field attributes
    prot = ((b->getByte())>>5)&1;
    bool num  = ((b->getByte())>>4)&1;
    int disppen = ((b->getByte())>>2)&3;
    bool mdt = (b->getByte())&1;

    askip = (prot & num);

    if(prot)
    {
        if (disppen == 2)
        {
            fieldAttr = palette[7]; //TODO: use names  (white)
        }
        else
        {
            fieldAttr = palette[1]; /* Blue */
        }

    }
    else
    {
        if (disppen == 2)
        {
            fieldAttr = palette[2]; /* Red */
        }
        else
        {
            fieldAttr = palette[4]; /* Green */
        }
    }
    printf("%02d,%02d: Attrbute byte: %02.2X prot=%d num=%d disppen=%d mdt=%d\n", primary_x, primary_y, b->getByte(), prot, num, disppen, mdt);
    fflush(stdout);

}

void DisplayDataStream::placeChar(Buffer *b)
{
    int ebcdic = (int)(b->getByte());

    placeChar(ebcdic);
}

void DisplayDataStream::placeChar(int ebcdic)
{

    int pos = primary_x + (primary_y * SCREENX);

    if (screenFields.find(pos) != screenFields.end())
    {
        printf("Removed existing field entry at %02d,%02d\n", primary_x, primary_y);
        fflush(stdout);
        screenFields.erase(pos);
    }

    glyph[pos]->setBrush(fieldAttr);

    switch(ebcdic)
    {
        case IBM3270_CHAR_NULL:
            glyph[pos]->setText(0x00);
            chars->setChar(pos, 0x00);
            break;
        default:
            if (extended.on)
            {
                glyph[pos]->setBrush(palette[extended.foreground]);
                cells[pos]->setBrush(palette[extended.background]);
            }
            glyph[pos]->setText(QString(EBCDICtoASCIImap[ebcdic]));
            attributes[pos].prot = prot;
    }

    if (++primary_x >= SCREENX)
    {
        primary_x = 0;
        if (++primary_y >= SCREENY)
        {
            primary_y = 0;
            printf("ERROR - screen overflow\n");
        }
    }
}

void DisplayDataStream::insertChar(QString keycode, bool insMode)
{
    int pos = cursor_x + (cursor_y * SCREENX);

    std::map<int, DisplayDataStream::FieldFlags>::iterator f = findField(pos);

    if (f->second.prot || f->first == pos)
    {
        printf("Protected!\n");
        fflush(stdout);
        return;
    }

    if (insMode)
    {
        std::map<int, DisplayDataStream::FieldFlags>::iterator fn = f;

        int endPos = (SCREENX * SCREENY) - 1;
        if (++fn != screenFields.end())
        {
            endPos = fn->first - 1;
        }
        bool space = false;
        for(int fld = pos; fld < endPos && !space; fld++)
        {
            if (glyph[fld]->text() == IBM3270_CHAR_NULL)
            {
                endPos = fld;
                space = true;
            }
        }
        if (!space)
        {
            printf("Overflow!\n");
            fflush(stdout);
            return;
        }
        for(int fld = endPos; fld > pos; fld--)
        {
            qDebug() << glyph[fld]->text();
            glyph[fld]->setText(glyph[fld-1]->text());
        }
    }

    f->second.mdt = true;

    glyph[pos]->setText(keycode);

    moveCursor(1, 0);
}

void DisplayDataStream::deleteChar()
{
    int pos = getCursorAddress();

    std::map<int, DisplayDataStream::FieldFlags>::iterator f = findField(pos);

    if (f->second.prot || f->first == pos)
    {
        printf("Protected!\n");
        fflush(stdout);
        return;
    }

    std::map<int, DisplayDataStream::FieldFlags>::iterator fn = f;

    int endPos = (SCREENX * SCREENY) - 1;

    if (++fn != screenFields.end())
    {
        endPos = fn->first - 1;
    }

    for(int fld = pos; fld < endPos; fld++)
    {
        qDebug() << glyph[fld]->text();
        glyph[fld]->setText(glyph[fld+1]->text());
    }

    glyph[endPos]->setText(IBM3270_CHAR_NULL);

    f->second.mdt = true;
}


void DisplayDataStream::eraseField()
{
    int cpos = getCursorAddress();

    /* Find current field, and next field */
    std::map<int, DisplayDataStream::FieldFlags>::iterator f = findField(cpos);
    std::map<int, DisplayDataStream::FieldFlags>::iterator fn;

    /* If there is no next field, find first field */
    if (f == screenFields.end())
    {
        fn = screenFields.begin();
    }
    else
    {
        fn = f;
        fn++;
    }

    /* Calculate ending position - either start of next field of end of screen */
    int fend;

    if (fn->first < f->first)
    {
        fend = (SCREENX * SCREENY);
    }
    else
    {
        fend = fn->first;
    }

    f->second.mdt = true;

    /* Blank field */
    for(int i = cpos; i < fend; i++)
    {
        glyph[i]->setText(0x00);
    }

    /* If there wasn't a next field, start again from the start of the screen */
    if (fn->first < f->first)
    {
        for(int i = 0; i < fn->first; i++)
        {
            glyph[i]->setText(0x00);
        }
    }
}

void DisplayDataStream::moveCursor(int x, int y, bool absolute)
{
    // Absolute or relative
    if (absolute)
    {
        cursor_x = x;
        cursor_y = y;
    }
    else
    {
        cursor_x+= x;
        cursor_y+= y;
    }

    printf("Cursor now: %d,%d\n", cursor_x, cursor_y);
    fflush(stdout);
    if(cursor_x >= SCREENX)
    {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_x < 0)
    {
        cursor_x = SCREENX - 1;
        cursor_y--;
    }
    if(cursor_y >= SCREENY)
    {
        cursor_y = 0;
    }
    if (cursor_y < 0)
    {
        cursor_y = SCREENY - 1;
    }
    addCursor();
}

void DisplayDataStream::addCursor()
{
    cursor->setParentItem(cells[cursor_x + (cursor_y * SCREENX)]);
    cursor->setPos(cells[cursor_x + (cursor_y * SCREENX)]->boundingRect().left(), cells[cursor_x + (cursor_y * SCREENX)]->boundingRect().top());

}

int DisplayDataStream::getCursorAddress(int offset)
{

    int c1 = cursor_x + (cursor_y * SCREENX);

    if (offset == 0)
    {
        return c1;
    }

    if ((c1 + offset) > (SCREENX * SCREENY) - 1)
    {
        return 0;
    }

    return c1;
}

void DisplayDataStream::showFields()
{
    int t = 0;
    for (std::map<int, FieldFlags>::iterator it = screenFields.begin(); it != screenFields.end(); it++)
    {
        FieldFlags thisField = it->second;
         printf("Field %d at %d (protected = %d, askip = %d)\n", t++, it->first, thisField.prot, thisField.askip);
    }
}

void DisplayDataStream::tab()
{
    FieldIterator it = findNextUnprotected();

    cursor_y = (it->first / SCREENX);
    cursor_x = it->first - (cursor_y * SCREENX);
    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);
}

void DisplayDataStream::home()
{
    FieldIterator it = findFirstUnprotected();
    cursor_y = (it->first / SCREENX);
    cursor_x = it->first - (cursor_y * SCREENX);

    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);
}


DisplayDataStream::FieldIterator DisplayDataStream::findNextUnprotected()
{
    int cpos = getCursorAddress();

    for (FieldIterator it = screenFields.begin(); it != screenFields.end(); it++)
    {
        if (it->first > cpos && !it->second.prot)
        {
            int nextPos = getCursorAddress(it->first + 1);
            if (screenFields.find(nextPos) == screenFields.end())
            {
                return it;
            }
        }
    }
    return findFirstUnprotected();

}

DisplayDataStream::FieldIterator DisplayDataStream::findFirstUnprotected()
{
    for (FieldIterator it = screenFields.begin(); it != screenFields.end(); it++)
    {
        if (!it->second.prot)
        {
            int nextPos = getCursorAddress(it->first + 1);
            if (screenFields.find(nextPos) == screenFields.end())
            {
                return it;
            }
        }
    }
    return screenFields.begin();
}

DisplayDataStream::FieldIterator DisplayDataStream::findField(int pos)
{
    FieldIterator last = screenFields.begin();

    for (std::map<int, FieldFlags>::iterator it = screenFields.begin(); it != screenFields.end(); it++)
    {
        if (it->first > pos)
        {
            printf("Field begins at %d\n", last->first);
            fflush(stdout);
            return last;
        }
        last = it;
    }
    return last;
}

Buffer *DisplayDataStream::processFields()
{
    Buffer *respBuffer = new Buffer();

    respBuffer->add(IBM3270_AID_ENTER);

    int cPos = getCursorAddress();

    respBuffer->add(twelveBitBufferAddress[(cPos>>5)&63]);
    respBuffer->add(twelveBitBufferAddress[cPos&63]);

    FieldIterator fn = screenFields.begin();

    for (FieldIterator it = screenFields.begin(); it != screenFields.end(); it++)
    {
        if (it->second.mdt)
        {
            fn = it;
            int endPos = (SCREENX * SCREENY) - 1;
            if (++fn != screenFields.end())
            {
                endPos = fn->first - 1;
            }

            respBuffer->add(IBM3270_SBA);
            int f = it->first + 1;


            respBuffer->add(twelveBitBufferAddress[(f>>6)&63]);
            respBuffer->add(twelveBitBufferAddress[(f&63)]);

            for(int i = f; i <= endPos; i++)
            {
                uchar b = glyph[i]->toUChar();
                if (b != IBM3270_CHAR_NULL)
                {
                    respBuffer->add(ASCIItoEBCDICmap[b]);
                }
            }
            it->second.mdt = false;
        }
    }

    respBuffer->dump();

    return respBuffer;
}

