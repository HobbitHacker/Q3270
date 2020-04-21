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

DisplayDataStream::DisplayDataStream(QGraphicsScene* parent, DisplayView *dv)
{
    //TODO: screen sizes

    scene = parent;
    view = dv;

    primary_x = 0;
    primary_y = 0;
    primary_pos = 0;

    cursor_x = 0;
    cursor_y = 0;
    cursor_pos = 0;

    default_screen = new DisplayData(parent, 80, 24);
    alternate_screen = new DisplayData(parent, 80, 43);

    setScreen();
}

void DisplayDataStream::setScreen(bool alternate)
{
    if (alternate)
    {
        view->setScene(alternate_screen->getScene());
        screen = alternate_screen;
    }
    else
    {
        view->setScene(default_screen->getScene());
        screen = default_screen;
    }

    alternate_size = alternate;

    screen_x = screen->width();
    screen_y = screen->height();

    screenSize = screen_x * screen_y;
}

void DisplayDataStream::processStream(Buffer *b)
{
    //FIXME: buffer size 0 shouldn't happen!
    if (b->size() == 0)
    {
        return;
    }

    b->dump();

    wsfProcessing = false;

    // Process WRITE command
    // Structured Fields Require Multiple WRITE commands
    // Add logic to cycle round buffer for structure field length if appropriate so we can come
    // back here to restart.
    screen->resetCharAttr();

    switch(b->getByte())
    {
        case IBM3270_EW:
        case IBM3270_CCW_EW:
            processEW(b, false);
            b->nextByte();
            break;
        case IBM3270_W:
        case IBM3270_CCW_W:
            processW(b);
            b->nextByte();
            break;
        case IBM3270_WSF:
        case IBM3270_CCW_WSF:
            wsfProcessing = true;
            b->nextByte();
            break;
        case IBM3270_EWA:
        case IBM3270_CCW_EWA:
            processEW(b, true);
            b->nextByte();
            break;
        default:
            printf("[Unrecognised WRITE command: %2.2X - Block Ignored]", b->getByte());
            b->dump();
            processing = false;
            return;
    }

    while(b->moreBytes())
    {
        if (wsfProcessing)
        {
            processWSF(b);
        }
        else
        {
            processOrders(b);
        }
        b->nextByte();
    }

    b->reset();
    b->setProcessing(false);

    if (resetMDT)
    {

    }
    screen->dumpFields();
//    screen->dumpDisplay();
    fflush(stdout);

}

void DisplayDataStream::processOrders(Buffer *b)
{
    switch(b->getByte())
    {
        case IBM3270_SF:
            processSF(b->nextByte());
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
        case IBM3270_EUA:
            processEUA(b);
            break;
        default:
            placeChar(b->getByte());
    }
}

void DisplayDataStream::processWCC(Buffer *b)
{
    std::string c;

    uchar wcc = b->getByte();

    int reset = (wcc>>6)&1;

    printf("WCC=%2.2X(",wcc);

    resetMDT = wcc&1;
    resetKB  = (wcc>>1)&1;
    alarm    = (wcc>>2)&1;

    if (reset)
    {
        printf("reset");
    }

    if (resetMDT)
    {
        if (reset)
        {
            printf(",");
        }
        printf("reset MDT");
    }

    if (resetKB)
    {
        if(reset|resetMDT)
        {
            printf(",");
        }
        printf("reset KB");
    }

    if (alarm)
    {
        if (resetKB|resetMDT|reset)
        {
            printf(",");
        }
        printf("alarm");
    }
    printf(")");
}

void DisplayDataStream::processEW(Buffer *buf, bool alternate)
{
    printf("[Erase Write ");

    if (alternate)
    {
        printf("Alternate ");
    }

    if (alternate != alternate_size)
    {
        setScreen(alternate);
    }

    processWCC(buf->nextByte());

    printf("]");
    fflush(stdout);

    primary_x = 0;
    primary_y = 0;
    primary_pos = 0;

    cursor_x = 0;
    cursor_y = 0;
    cursor_pos = 0;

//    display->setBackgroundBrush(Qt::black);

    screen->clear();

}

void DisplayDataStream::processW(Buffer *buf)
{
    printf("[Write ");

    processWCC(buf->nextByte());

    printf("]");
    fflush(stdout);
}

void DisplayDataStream::processSF(Buffer *buf)
{
    printf("[Start Field:");

    screen->setField(primary_pos, buf->getByte(), false);

    printf("]");
    fflush(stdout);

    incPos();
}

void DisplayDataStream::processSBA(Buffer *buf)
{
    printf("[SetBufferAddress ");
    primary_pos = extractBufferAddress(buf->nextByte());

    primary_y = (primary_pos / screen_x);
    primary_x = primary_pos - (primary_y * screen_x);
    printf(" %d,%d (%d)]", primary_x, primary_y, primary_pos);
}

void DisplayDataStream::processSFE(Buffer *b)
{
    int pairs = b->nextByte()->getByte();

    int type;
    int value;

    screen->resetExtended(primary_pos);

    printf("[SFE pairs %d]", pairs);
    for(int i = 1; i <= pairs; i++)
    {
        type = b->nextByte()->getByte();
        value = b->nextByte()->getByte();

//        printf("%3.3d  Type: %2.2X = %2.2X ",i,  type, value);
        switch(type)
        {
            case IBM3270_EXT_3270:
                printf("[Field ");
                screen->setField(primary_pos, b->getByte(), true);
                printf("]");
                break;
            case IBM3270_EXT_FG_COLOUR:
                printf("Extended FG");
                screen->setExtendedColour(primary_pos, true, value);
                break;
            case IBM3270_EXT_BG_COLOUR:
                printf("Extended BG");
                screen->setExtendedColour(primary_pos, false, value);
                break;
            case IBM3270_EXT_HILITE:
                switch(value)
                {
                    case IBM3270_EXT_HI_NORMAL:
                        printf("[Reset Extended]");
                        screen->resetExtendedHilite(primary_pos);
                        break;
                    case IBM3270_EXT_HI_BLINK:
                        screen->setExtendedBlink(primary_pos);
                        break;
                    case IBM3270_EXT_HI_REVERSE:
                        screen->setExtendedReverse(primary_pos);
                        break;
                    case IBM3270_EXT_HI_USCORE:
                        screen->setExtendedUscore(primary_pos);
                        break;
                    default:
                        break;
                }
                break;
            default:
                printf("[%2.2X-%2.2X ***Ignored***]", type, value);
                break;
        }
        fflush(stdout);
    }
    screen->setFieldAttrs(primary_pos);
    incPos();
}

void DisplayDataStream::processIC()
{
    moveCursor(primary_x, primary_y, true);
}

void DisplayDataStream::processRA(Buffer *b)
{
    int endPos = extractBufferAddress(b->nextByte());

    if (endPos > screenSize - 1)
    {
        endPos = screenSize - 1;
    }

    b->nextByte();
    uchar newChar = b->getByte();

    printf("[RepeatToAddress %d to %d (0x%2.2X)]", primary_pos, endPos, newChar);
    fflush(stdout);

    if (endPos < primary_pos)
    {
        endPos += screenSize;
    }

    for(int i = primary_pos; i < endPos; i++)
    {
        int offset = i % screenSize;

        screen->setChar(offset, newChar);
    }

    primary_pos = endPos % screenSize;
    primary_y = (primary_pos / screen_x);
    primary_x = primary_pos - (primary_y * screen_x);
}

void DisplayDataStream::processSA(Buffer *b)
{
    int extendedType = b->nextByte()->getByte();
    int extendedValue = b->nextByte()->getByte();
    screen->setCharAttr(extendedType, extendedValue);
}

void DisplayDataStream::processWSF(Buffer *b)
{
    wsfProcessing = true;

    wsfLen = b->getByte()<<8;
    b->nextByte();
    wsfLen += b->getByte();
    b->nextByte();

    printf("WSF - length: %03d; command = %2.2X\n", wsfLen, b->getByte());
    fflush(stdout);

    // Decrease length by two-byte length field and structured field id byte
    wsfLen-=3;

    switch(b->getByte())
    {
        case IBM3270_WSF_RESET:
            WSFreset(b);
            break;
        case IBM3270_WSF_READPARTITION:
            WSFreadPartition(b);
            break;
        case IBM3270_WSF_OB3270DS:
            WSFoutbound3270DS(b);
            break;
        default:
            printf("Unimplemented WSF command: %2.2X\n", b->getByte());
            break;
    }
}

void DisplayDataStream::processEUA(Buffer *b)
{
    printf("[EraseUnprotected to Address ");
    int stopAddress = extractBufferAddress(b->nextByte());
    printf("]");

    screen->eraseUnprotected(primary_pos, stopAddress);
}

void DisplayDataStream::WSFoutbound3270DS(Buffer *b)
{
    printf("[Outbound 3270DS");
    int partition = b->nextByte()->getByte();

    printf("partition #%d ", partition);

    int cmnd = b->nextByte()->getByte();

    wsfLen-=2;

    switch(cmnd)
    {
        case IBM3270_W:
            printf("Write ");
            processWCC(b->nextByte());
            wsfLen--;
            break;
        default:
            printf("** Unrecognised command %d **]", cmnd);
    }
    while(wsfLen>0)
    {
        processOrders(b);
        wsfLen--;
    }
}

void DisplayDataStream::WSFreset(Buffer *b)
{
    printf("\n\nReset Partition (***Not Implemented***) %2.2X\n\n", b->nextByte()->getByte());
    fflush(stdout);
    return;
}

void DisplayDataStream::WSFreadPartition(Buffer *b)
{
    uchar partition = b->nextByte()->getByte();
    uchar type = b->nextByte()->getByte();

    printf("ReadPartition %d - type %2.2X\n", partition, type);

    Buffer *queryReply = new Buffer();
    queryReply->add(IBM3270_WSF_QUERYREPLY);

    replySummary(queryReply);

    emit(bufferReady(queryReply));

}

void DisplayDataStream::replySummary(Buffer *buffer)
{

    /*

0040         88 00 0e 81 80 80 81 84 85 86 87 88 95 a1
0050   a6 00 17 81 81 01 00 00 50 00 2b 01 00 0a 02 e5
0060   00 02 00 6f 09 0c 0d 70 00 08 81 84 00 0d 70 00
0070   00 1b 81 85 82 00 09 0c 00 00 00 00 07 00 10 00
0080   02 b9 00 25 01 10 f1 03 c3 01 36 00 26 81 86 00
0090   10 00 f4 f1 f1 f2 f2 f3 f3 f4 f4 f5 f5 f6 f6 f7
00a0   f7 f8 f8 f9 f9 fa fa fb fb fc fc fd fd fe fe ff
00b0   ff ff ff 00 0f 81 87 05 00 f0 f1 f1 f2 f2 f4 f4
00c0   f8 f8 00 07 81 88 00 01 02 00 0c 81 95 00 00 10
00d0   00 10 00 01 01 00 12 81 a1 00 00 00 00 00 00 00
00e0   00 06 a7 f3 f2 f7 f0 00 11 81 a6 00 00 0b 01 00
00f0   00 50 00 18 00 50 00 2b ff ef

     */
//    unsigned char qrt[] = { 0x81, 0x80, 0x86, 0x87, 0xA6 0x87 };
    unsigned char qrt[] = { 0x81, 0x86, 0xA6 };
    unsigned char qrcolour[] = { 0x81, 0x86, 0x00, 0x08,
                                 0x00, 0xF4,  /* Default colour */
                                 0xF1, 0xF1,  /* Blue */
                                 0xF2, 0xF2,  /* Red */
                                 0xF3, 0xF3,  /* Magenta */
                                 0xF4, 0xF4,  /* Green */
                                 0xF5, 0xF5,  /* Cyan */
                                 0xF6, 0xF6,  /* Yellow */
                                 0xF7, 0xf7   /* White */
                               };
    unsigned char qpart[] = { 0x81, 0xA6, 0x00, 0x00, 0x0B, 0x01, 0x00, 0x00, 0x50, 0x00, 0x18, 0x00, 0x50, 0x00, 0x2B
                            };

    unsigned char qhighlight[] = { 0x81, 0x87, 0x04, 0x00, 0xF0, 0xF1, 0xF1, 0xF2, 0xF2, 0xF4, 0xF4};

    buffer->add(0x00);    //(char*)qrt)>>8);
    buffer->add(0x05);    //strlen((char*)qrt)&0xFF);

    for(int i = 0; (unsigned long)i < 3; i++)
    {
        buffer->add(qrt[i]);
    }

    buffer->add(0x00);
    buffer->add(22);

    for(int i = 0; (unsigned long)i < 20; i++)
    {
        buffer->add(qrcolour[i]);
    }

    buffer->add(0x00);
    buffer->add(17);

    for(int i = 0; (unsigned long)i < 15; i++)
    {
        buffer->add(qpart[i]);
    }

    buffer->add(0x00);
    buffer->add(13);

    for(int i = 0; (unsigned long)i < 11; i++)
    {
        buffer->add(qhighlight[i]);
    }


}

int DisplayDataStream::extractBufferAddress(Buffer *b)
{
    //TODO: non-12/14 bit addresses & EBCDIC characters

    unsigned char sba1 = b->getByte();
    unsigned char sba2 = b->nextByte()->getByte();

    switch((sba1>>6)&3)
    {
        case 0b11:     // 12 bit
            printf("%2.2X%2.2X (%d%d - 12 bit)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return ((sba1&63)<<6)+(sba2&63);
        case 0b01:     // 12 bit
            printf("%2.2X%2.2X (%d%d - 12 bit)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return ((sba1&63)<<6)+(sba2&63);
        case 0b00:     // 14 bit
            printf("%2.2X%2.2X (%d%d - 14 bit)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return ((sba1&63)<<8)+sba2;
        case 0b10:     // reserved
            printf("%2.2X%2.2X (%d%d - unknown)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return 0;
    }
    printf("Extract Buffer Address failed: sba1: %2.2X sba2: %2.2X (sba1&63 = %d%d)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);

    return -1;
}

void DisplayDataStream::placeChar(Buffer *b)
{
    int ebcdic = (int)(b->getByte());

    placeChar(ebcdic);
}

void DisplayDataStream::placeChar(int ebcdic)
{

//    glyph[pos]->setBrush(fieldAttr);

    switch(ebcdic)
    {
        case IBM3270_CHAR_NULL:
            screen->setChar(primary_pos, 0x00);
            break;
        default:
/*            if (extended.on)
            {
                glyph[pos]->setBrush(palette[extended.foreground]);
                cells[pos]->setBrush(palette[extended.background]);
            }*/
            screen->setChar(primary_pos, ebcdic);
//            attributes[pos].prot = prot;
    }

    incPos();
}

void DisplayDataStream::incPos()
{
    primary_pos++;
    if (++primary_x >= screen_x)
    {
        primary_x = 0;
        if (++primary_y >= screen_y)
        {
            primary_pos = 0;
            primary_y = 0;
        }
    }
}

void DisplayDataStream::insertChar(unsigned char keycode, bool insMode)
{
    if (screen->insertChar(cursor_pos, keycode, insMode))
    {
        moveCursor(1, 0);
    }
}

void DisplayDataStream::deleteChar()
{
    screen->deleteChar(cursor_pos);
}


void DisplayDataStream::eraseField()
{
    screen->eraseEOF(cursor_pos);
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

        if(cursor_x >= screen_x)
        {
            cursor_x = 0;
            cursor_y++;
        }
        if (cursor_x < 0)
        {
            cursor_x = screen_x - 1;
            cursor_y--;
        }
        if(cursor_y >= screen_y)
        {
            cursor_y = 0;
        }
        if (cursor_y < 0)
        {
            cursor_y = screen_y - 1;
        }
    }

//    printf("Cursor now %d,%d\n", cursor_x, cursor_y);
    fflush(stdout);

    cursor_pos = cursor_x + (cursor_y * screen_x);
    screen->setCursor(cursor_pos);
}

void DisplayDataStream::tab()
{
    int nf = screen->findNextUnprotectedField(cursor_pos);

    if(nf == cursor_pos)
    {
        return;
    }

    cursor_y = (nf / screen_x);
    cursor_x = nf - (cursor_y * screen_x);
    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);
}

void DisplayDataStream::home()
{
    int nf = screen->findNextUnprotectedField(0);
    cursor_y = (nf / screen_x);
    cursor_x = nf - (cursor_y * screen_x);

    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);
}

void DisplayDataStream::newline()
{
    cursor_x = 0;
    cursor_y += 1;

    if (cursor_y > screen_y)
    {
        cursor_y = 0;
    }

    cursor_pos = cursor_x + cursor_y * screen_x;

    tab();
}


Buffer *DisplayDataStream::processFields(int aid)
{
    Buffer *respBuffer = new Buffer();

    respBuffer->add(aid);

    respBuffer->add(0xC0|((cursor_pos>>6)&63));
    respBuffer->add(cursor_pos&63);

    screen->getModifiedFields(respBuffer);

    respBuffer->dump();

    return respBuffer;
}

