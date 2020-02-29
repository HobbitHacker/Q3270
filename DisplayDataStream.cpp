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

    //TODO: return codes from malloc
    buffer = (uchar*) malloc(102400);
    bufferCurrentPos = buffer;

    bufferLength = 0;

    //TODO: screen sizes
    defaultScreenSize = 80*24;
    alternateScreenSize = 80*24;

    primary_x = 0;
    primary_y = 0;

    cursor_x = 0;
    cursor_y = 0;

    count = 0;

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
}

void DisplayDataStream::addByte(uchar b)
{
    *bufferCurrentPos++ = b;
    bufferLength++;
}

void DisplayDataStream::processStream()
{
    uchar *b = buffer;

    if (*b == (uchar)IBM3270_EW)
    {
        b = processEW(b);
    }
    else
    {
        printf("Eeek. Structured field!\n");
    }

    while(b < (buffer + bufferLength))
    {
//        printf("Processing .. BufferLenth=%d (bufferCurrentPos=%d)\n", bufferLength, bufferCurrentPos);
        switch(*b)
        {
            case IBM3270_SF:
                b = processSF(b);
                break;
            case IBM3270_SBA:
                b = processSBA(b);
                break;
            case IBM3270_SFE:
                b = processSFE(b);
                break;
            case IBM3270_IC:
                processIC();
                break;
            case IBM3270_RA:
                b = processRA(b);
                break;
            case IBM3270_W:  // TODO: cursor address for WRITE command
            default:
                placeChar(b);
        }
        b++;
    }
    processing = false;
    bufferLength = 0;
    bufferCurrentPos = buffer;

}

uchar * DisplayDataStream::processEW(uchar *buf)
{
    uchar wcc = *(++buf);

    bool resetMDT = !(wcc&1);
    resetKB  = (wcc>>1)&1;
    alarm    = (wcc>>2)&1;

    printf("Seen WCC %02.2X\n", (uchar) wcc);

    primary_x = 0;
    primary_y = 0;

    cursor_x = 0;
    cursor_y = 0;

//    display->setBackgroundBrush(Qt::black);

    for(int i = 0; i < SCREENX * SCREENY; i++)
    {
        cells[i]->setBrush(Qt::black);
        glyph[i]->setBrush(Qt::green);
        glyph[i]->setText(0x00);
    }

    screenFields.clear();

    return ++buf;

}

uchar * DisplayDataStream::processSF(uchar *buf)
{
    buf++;

    int pos = primary_x + (primary_y * SCREENX);

    setAttributes(buf);

    screenFields.emplace(pos, FieldFlags{ askip, prot });

    uchar spc = 0x40; // EBCDIC space :-)
    placeChar(&spc);

    return buf;
}

uchar *DisplayDataStream::processSBA(uchar *buf)
{
    int pos = extractBufferAddress(++buf);

    primary_y = (pos / SCREENX);
    primary_x = pos - (primary_y * SCREENX);

    return ++buf;
}

uchar *DisplayDataStream::processSFE(uchar *b)
{
    printf("SFE seen\n");
    return ++b;
}

void DisplayDataStream::processIC()
{
    cursor_x = primary_x;
    cursor_y = primary_y;
    addCursor();
}

uchar *DisplayDataStream::processRA(uchar *b)
{
    int endPos = extractBufferAddress(++b);

    if (endPos > (SCREENX * SCREENY) - 1)
    {
        endPos = (SCREENX * SCREENY) - 1;
    }

    int curPos = primary_x + (primary_y * SCREENX);

    b++;
    uchar *newChar = ++b;

    if (endPos > curPos)
    {
        for(int i = curPos; i < endPos; i++)
        {
            placeChar(newChar);
        }
    }
    if (endPos < curPos)
    {
        for(int i = 0; i < endPos; i++)
        {
            placeChar(newChar);
        }
    }

    return b;
}

int DisplayDataStream::extractBufferAddress(uchar *b)
{
    //TODO: non-12/14 bit addresses
    int sba1 = *b++;
    int sba2 = *b;

    sba1 = sba1&63;
    sba2 = sba2&63;

    return (sba1<<6)|sba2;
}

void DisplayDataStream::setAttributes(uchar *b)
{
    bool prot = ((*b)>>5)&1;
    bool num  = ((*b)>>4)&1;
    int disppen = ((*b)>>2)&3;
    bool mdt = (*b)&1;

    askip = (prot & num);

    if(prot)
    {
        this->prot = true;
        if (disppen == 2)
        {
            fieldAttr = Qt::white;
        }
        else
        {
            fieldAttr = Qt::blue;
        }

    }
    else
    {
        this->prot = false;
        if (disppen == 2)
        {
            fieldAttr = Qt::red;
        }
        else
        {
            fieldAttr = Qt::green;
        }
    }

}

void DisplayDataStream::placeChar(uchar *b)
{
    int pos = primary_x + (primary_y * SCREENX);

    int ebcdic = (int)((uchar) *b);

    switch(ebcdic)
    {
        case IBM3270_CHAR_NULL:
            glyph[pos]->setText(0x00);
            break;
        default:
            glyph[pos]->setText(QString(EBCDICtoASCIImap[ebcdic]));
    }

    glyph[pos]->setBrush(fieldAttr);

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
        QString currentField;
        if (glyph[endPos]->text() != IBM3270_CHAR_NULL && glyph[pos]->text() != IBM3270_CHAR_SPACE)
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
    for(int i = f->first + 1; i < fend; i++)
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

int DisplayDataStream::getCursorAddress()
{

    int c1 = cursor_x + (cursor_y * SCREENX);

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

void DisplayDataStream::findNextUnprotected()
{
    int cpos = getCursorAddress();

    for (std::map<int, FieldFlags>::iterator it = screenFields.begin(); it != screenFields.end(); it++)
    {
        if (it->first > cpos && !it->second.prot)
        {
            cursor_y = (it->first / SCREENX);
            cursor_x = it->first - (cursor_y * SCREENX);
            moveCursor(1, 0);
            return;
        }
    }
    findFirstUnprotected();

}

void DisplayDataStream::findFirstUnprotected()
{
    for (std::map<int, FieldFlags>::iterator it = screenFields.begin(); it != screenFields.end(); it++)
    {
        if (!it->second.prot)
        {
            cursor_y = (it->first / SCREENX);
            cursor_x = it->first - (cursor_y * SCREENX);
            moveCursor(1, 0);
            return;
        }
    }
}

std::map<int, DisplayDataStream::FieldFlags>::iterator DisplayDataStream::findField(int pos)
{
    std::map<int, FieldFlags>::iterator last = screenFields.begin();

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

