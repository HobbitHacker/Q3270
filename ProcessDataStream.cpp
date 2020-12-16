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

#include "ProcessDataStream.h"

ProcessDataStream::ProcessDataStream(QGraphicsScene* parent, Terminal *t)
{
    //TODO: some of this stuff belongs in Terminal()

    scene = parent;
    terminal = t;

    primary_x = 0;
    primary_y = 0;
    primary_pos = 0;

    cursor_x = 0;
    cursor_y = 0;
    cursor_pos = 0;

    lastAID = IBM3270_AID_NOAID;

    default_screen = new DisplayScreen(parent, t, 80, 24);
    alternate_screen = new DisplayScreen(parent, t, t->terminalWidth(), t->terminalHeight());

    setScreen();
}

void ProcessDataStream::setScreen(bool alternate)
{
    if (alternate)
    {
        terminal->setScene(alternate_screen->getScene());
        screen = alternate_screen;
    }
    else
    {
        terminal->setScene(default_screen->getScene());
        screen = default_screen;
    }

    alternate_size = alternate;

    screen_x = screen->width();
    screen_y = screen->height();

    screenSize = screen_x * screen_y;
}

void ProcessDataStream::setFont(QFont font)
{
    default_screen->setFont(font);
    alternate_screen->setFont(font);
}

void ProcessDataStream::processStream(Buffer *b)
{
    //FIXME: buffer size 0 shouldn't happen!
    if (b->size() == 0)
    {
        printf("DisplayDataStream: zero buffer size - returning\n");
        fflush(stdout);
        return;
    }

    b->dump();

    wsfProcessing = false;

    // Process Command codes
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
        case IBM3270_RM:
        case IBM3270_CCW_RM:
            processRM();
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
    if (resetKB)
    {
        printf("WCC reset keyboard - ");
        emit(keyboardUnlocked());
    }
    screen->dumpFields();
//    screen->dumpDisplay();
    fflush(stdout);

}

void ProcessDataStream::processOrders(Buffer *b)
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
        case IBM3270_GE:
            processGE(b);
            break;
        default:
            placeChar(b->getByte());
    }
}

void ProcessDataStream::processWCC(Buffer *b)
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
        lastAID = IBM3270_AID_NOAID;
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


void ProcessDataStream::processW(Buffer *buf)
{
    printf("[Write ");

    processWCC(buf->nextByte());

    printf("]");
    fflush(stdout);
}

void ProcessDataStream::processEW(Buffer *buf, bool alternate)
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

void ProcessDataStream::processWSF(Buffer *b)
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

void ProcessDataStream::processRM()
{
    processAID(lastAID, false);
}

void ProcessDataStream::processSF(Buffer *buf)
{
    printf("[Start Field:");

    screen->setField(primary_pos, buf->getByte(), false);

    printf("]");
    fflush(stdout);

    incPos();
}

void ProcessDataStream::processSFE(Buffer *b)
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

void ProcessDataStream::processSBA(Buffer *buf)
{
    printf("[SetBufferAddress ");
    primary_pos = extractBufferAddress(buf->nextByte());

    primary_y = (primary_pos / screen_x);
    primary_x = primary_pos - (primary_y * screen_x);
    printf(" %d,%d (%d)]", primary_x, primary_y, primary_pos);
}

void ProcessDataStream::processSA(Buffer *b)
{
    int extendedType = b->nextByte()->getByte();
    int extendedValue = b->nextByte()->getByte();
    screen->setCharAttr(extendedType, extendedValue);
}

void ProcessDataStream::processIC()
{
    moveCursor(primary_x, primary_y, true);
}

void ProcessDataStream::processRA(Buffer *b)
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

        screen->setChar(offset, newChar, false);
    }

    primary_pos = endPos % screenSize;
    primary_y = (primary_pos / screen_x);
    primary_x = primary_pos - (primary_y * screen_x);
}

void ProcessDataStream::processEUA(Buffer *b)
{
    printf("[EraseUnprotected to Address ");
    int stopAddress = extractBufferAddress(b->nextByte());
    printf("]");

    screen->eraseUnprotected(primary_pos, stopAddress);
    resetKB = true;
}

void ProcessDataStream::processGE(Buffer *b)
{
    screen->setGraphicEscape();
}

void ProcessDataStream::WSFoutbound3270DS(Buffer *b)
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

void ProcessDataStream::WSFreset(Buffer *b)
{
    printf("\n\nReset Partition (***Not Implemented***) %2.2X\n\n", b->nextByte()->getByte());
    fflush(stdout);
    return;
}

void ProcessDataStream::WSFreadPartition(Buffer *b)
{
    uchar partition = b->nextByte()->getByte();
    uchar type = b->nextByte()->getByte();

    printf("ReadPartition %d - type %2.2X\n", partition, type);

    Buffer *queryReply = new Buffer();
    queryReply->add(IBM3270_AID_SF);

    replySummary(queryReply);

    emit(bufferReady(queryReply));

}

void ProcessDataStream::replySummary(Buffer *buffer)
{

    /* 62 x 160

    > 0x0   88 000e 81 80 80 81 84 85 86 87 88 95 a1 a6 0017 81 81 01 00 *00a0* *003e* 01 000a 02e5 0002
    > 0x20  006f 09 0c *26c0* 000881840026c000001b81858200090c000000000700100002b9
    > 0x40  00250110f103c3013600268186001000f4f1f1f2f2f3f3f4f4f5f5f6f6f7f7f8
    > 0x60  f8f9f9fafafbfbfcfcfdfdfefeffffffff000f81870500f0f1f1f2f2f4f4f8f8
    > 0x80  00078188000102000c81950000100010000101001281a1000000000000000006
    > 0xa0  a7f3f2f7f0001181a600000b01000050001800a0003effef

    */

    /* 43 x 80

0040   88 - AID
       00 0e - length (14 bytes including length)
       81 - reply summary
       80 - summary query reply
       80 81 84 85 86 87 88 95 a1 - codes supported (summary, usable area, alphanumeric partitions, char sets, colour, hilighting, reply modes, DDM, RPQ names,
0050   a6 - implicit partitions)

       00 17 - length (23 bytes)
       81 - query reply
       81 - usable area
       01 - 12/14 bit addressing allowed
       00 - variable cells not supported, matrix character, units in cells
       00 50 - width of usable area
       00 2b - height of usable area
       01 - size in mm
       00 0a 02 e5 - distance between points in X as 2 byte numerator and 2 byte denominator (10/741)
0060   00 02 00 6f - distance between points in Y as 2 byte n/d (2/111)
       09 - number X of UNITS in default cell (9)
       0c - number of Y UNITS in default cell (12)
       0d 70 - display size (3440 - 43x80)

       00 08 - length
       81 - query reply
       84 - alphanumeric partitions
       00  - one partition only
       0d 70 - total partition storage (3440 - 43x80)
       00 - no vertical scrolling, no all points addressability, no partition protection, no presentation space copy, no modify partition

0070   00 1b - length
       81 - query reply
       85 - character sets
       82 - graphic escape, no multiple LCIDs, no LOAD PS, no LOAD PS EXTENDED, one char size only, no DBCS, not CGCSID
       00 - LOAD PS slot size required
       09 - default width
       0c - default height
       00 00 00 00 - LOAD PS format types supported (none)
          07  - char set 7
          00 -  non-loadable, single plane, single byte char set, LCID compare
          10 - local char set id
          00
0080      02 - char set 2
          b9 00 25 01 10 f1 03 c3 01 36

       00 26 - length (38)
       81  - query reply
       86 - colour
       00
0090   10 00 f4 f1 f1 f2 f2 f3 f3 f4 f4 f5 f5 f6 f6 f7
00a0   f7 f8 f8 f9 f9 fa fa fb fb fc fc fd fd fe fe ff
00b0   ff ff ff


       00 0f - length (15)
       81 - query reply
       87 - highlight
       05 00 f0 f1 f1 f2 f2 f4 f4
00c0   f8 f8

       00 07 - length (7)
       81 - query reply
       88 - reply modes
       00 - field mode
       01 - extended field mode
       02 - character mode

       00 0c - length (12)
       81 - query reply
       95 - DDM
       00 00 - reserved
       10
00d0   00 - limin 4096
       10 00 - limout 4096
       01 - 1 subset
       01 - subset id

       00 12 - length (18)
       81 - query reply
       a1 - rpq names
       00 00 00 00 - device type id
       00 00 00 00 - model (all models)
00e0   06 - length (6)
       a7 f3 f2 f7 f0 - x3270

       00 11 - length (17)
       81 - query reply
       a6 - implicit partition
       00 00 - reserved
       0b - length
       01 - implicit partition sizes
       00 - reserved
00f0   00 50 - width of default screen (80)
       00 18 - height of default screen (24)
       00 50 - width of alternate screen (80)
       00 2b - height of alternate screen (43)

     */
//    unsigned char qrt[] = { 0x81, 0x80, 0x86, 0x87, 0xA6 0x87 };
    unsigned char qrt[] = {
                            IBM3270_SF_QUERYREPLY,
                            IBM3270_SF_QUERYREPLY_SUMMARY,
                            IBM3270_SF_QUERYREPLY_COLOUR,
                            IBM3270_SF_QUERYREPLY_IMPPARTS,
                            IBM3270_SF_QUERYREPLY_USABLE
                          };

    unsigned char qrcolour[] = {
                                 IBM3270_SF_QUERYREPLY,
                                 IBM3270_SF_QUERYREPLY_COLOUR,
                                 0x00,        /* Flags:  x.xxxxxx - Reserved
                                                         .0...... - Printer Only - Black ribbon not loaded
                                                         .1...... - Printer Only - Black ribbon loaded
                                              */
                                 0x08,        /* Number of colours, plus default colour */
                                 0x00, 0xF4,  /* Default colour */
                                 0xF1, 0xF1,  /* Blue */
                                 0xF2, 0xF2,  /* Red */
                                 0xF3, 0xF3,  /* Magenta */
                                 0xF4, 0xF4,  /* Green */
                                 0xF5, 0xF5,  /* Cyan */
                                 0xF6, 0xF6,  /* Yellow */
                                 0xF7, 0xF7   /* White */
                               };

    unsigned char qpart[] = {
                              IBM3270_SF_QUERYREPLY,
                              IBM3270_SF_QUERYREPLY_IMPPARTS,
                              0x00, 0x00,  /* Reserved */
                              0x0B,  /* Data Length */
                              0x01,  /* Implicit Partition Sizes */
                              0x00,  /* Reserved */
                              0x00, 0x50,  /* Default Width in characters */
                              0x00, 0x18,  /* Default Height in characters */
                              0x00, 0x00,  /* Alternate Width in characters */
                              0x00, 0x00   /* Alternate Height in characters */
                            };

    qpart[12] = alternate_screen->width();
    qpart[14] = alternate_screen->height();

    unsigned char qhighlight[] = {
                                    IBM3270_SF_QUERYREPLY,
                                    IBM3270_SF_QUERYREPLY_HIGHLIGHT,
                                    0x04,  /* Number of pairs */
                                    0x00, 0xF0,  /* Default */
                                    0xF1, 0xF1,  /* Blink */
                                    0xF2, 0xF2,  /* Reverse */
                                    0xF4, 0xF4   /* Uscore */
                                 };

    unsigned char qusablearea[] = {
                                    IBM3270_SF_QUERYREPLY,
                                    IBM3270_SF_QUERYREPLY_USABLE,
                                    0x01,       /* 12/14 bit addressing */
                                    0x00,       /* Nothing enabled */
                                    0x00, 0x00, /*  4 & 5 - Columns */
                                    0x00, 0x00, /*  6 & 7 - Rows */
                                    0x01,       /* Units in mm */
                                    0x00, 0x00, /*  9 & 10 - Xr Numerator */
                                    0x00, 0x00, /* 11 & 12 - Xr Denominator */
                                    0x00, 0x00, /* 13 & 14 - Yr Numerator */
                                    0x00, 0x00, /* 15 & 16 - Yr Denominator */
                                    0x00, 0x00, /* 17 & 18 - X units in cell */
                                    0x00, 0x00, /* 19 & 20 - Y units in cell */
                                    0x00, 0x00  /* 21 & 22 - Screen buffer size */
                                  };

    buffer->add(0x00);    //(char*)qrt)>>8);
    buffer->add(0x06);    //strlen((char*)qrt)&0xFF);

    for(int i = 0; (unsigned long)i < 4; i++)
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

    qusablearea[4] = 0x00;
    qusablearea[5] = alternate_screen->width();

    qusablearea[6] = 0x00;
    qusablearea[7] = alternate_screen->height();

    QSizeF screenSizeMM = QGuiApplication::primaryScreen()->physicalSize();
    QSizeF screenSizePix = QGuiApplication::primaryScreen()->size();

    int xmm = screenSizeMM.width();
    int ymm = screenSizeMM.height();

    qusablearea[9]  = (xmm & 0xFF00) >> 8;
    qusablearea[10] = (xmm & 0xFF);

    qusablearea[13] = (ymm & 0xFF00) >> 8;
    qusablearea[14] = (ymm & 0xFF);

    int x = screenSizePix.width();
    int y = screenSizePix.height();

    qusablearea[11] = (x & 0xFF00) >> 8;
    qusablearea[12] = (x & 0xFF);

    qusablearea[15] = (y & 0xFF00) >> 8;
    qusablearea[16] = (y & 0xFF);

    qusablearea[17] = (default_screen->gridWidth() & 0xFF00) >> 8;
    qusablearea[18] = (default_screen->gridWidth() & 0xFF);

    qusablearea[19] = (default_screen->gridHeight() & 0xFF00) >> 8;
    qusablearea[20] = (default_screen->gridHeight() & 0xFF);

    qusablearea[21] = ((qusablearea[5] * qusablearea[7]) & 0xFF00) >> 8;
    qusablearea[22] = ((qusablearea[5] * qusablearea[7]) & 0xFF);

    buffer->add(0x00);
    buffer->add(25);

    for(int i = 0; (unsigned long)i < 23; i++)
    {
        buffer->add(qusablearea[i]);
    }


}

int ProcessDataStream::extractBufferAddress(Buffer *b)
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

void ProcessDataStream::placeChar(Buffer *b)
{
    int ebcdic = (int)(b->getByte());

    placeChar(ebcdic);
}

void ProcessDataStream::placeChar(int ebcdic)
{

//    glyph[pos]->setBrush(fieldAttr);

    switch(ebcdic)
    {
        case IBM3270_CHAR_NULL:
            screen->setChar(primary_pos, 0x00, false);
            break;
        default:
/*            if (extended.on)
            {
                glyph[pos]->setBrush(palette[extended.foreground]);
                cells[pos]->setBrush(palette[extended.background]);
            }*/
            screen->setChar(primary_pos, ebcdic, false);
//            attributes[pos].prot = prot;
    }

    incPos();
}

void ProcessDataStream::incPos()
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

void ProcessDataStream::insertChar(unsigned char keycode, bool insMode)
{
    if (screen->insertChar(cursor_pos, keycode, insMode))
    {
        moveCursor(1, 0);
    }
}

void ProcessDataStream::deleteChar()
{
    screen->deleteChar(cursor_pos);
}


void ProcessDataStream::eraseField()
{
    screen->eraseEOF(cursor_pos);
}

void ProcessDataStream::moveCursor(int x, int y, bool absolute)
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

    cursor_pos = cursor_x + (cursor_y * screen_x);

    printf("moveCursor: Now at %d,%d (%d) ", cursor_x, cursor_y, cursor_pos);
    screen->dumpAttrs(cursor_pos);
    fflush(stdout);

    screen->setCursor(cursor_pos);
    screen->drawRuler(cursor_x, cursor_y);

    emit cursorMoved(cursor_x, cursor_y);
}

void ProcessDataStream::tab(int offset)
{
    int nf = screen->findNextUnprotectedField(cursor_pos + offset);

/*    if(nf == cursor_pos)
    {
        return;
    }
*/
    cursor_y = (nf / screen_x);
    cursor_x = nf - (cursor_y * screen_x);
    printf("Unprotected field found at %d (%d,%d) ", nf, cursor_x, cursor_y);
    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);
}

void ProcessDataStream::backtab()
{
    int pf = screen->findPrevUnprotectedField(cursor_pos);

    cursor_y = (pf / screen_x);
    cursor_x = pf - (cursor_y * screen_x);
    printf("Backtab: Unprotected field found at %d (%d,%d) ", pf, cursor_x, cursor_y);
    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);

}

void ProcessDataStream::home()
{
    int nf = screen->findNextUnprotectedField(0);
    cursor_y = (nf / screen_x);
    cursor_x = nf - (cursor_y * screen_x);

    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);
}

void ProcessDataStream::newline()
{
    cursor_x = 0;
    cursor_y += 1;

    if (cursor_y > screen_y)
    {
        cursor_y = 0;
    }

    cursor_pos = cursor_x + cursor_y * screen_x;

    tab(0);
}

void ProcessDataStream::toggleRuler()
{
    screen->toggleRuler();
    screen->drawRuler(cursor_x, cursor_y);
}


void ProcessDataStream::processAID(int aid, bool shortRead)
{
    Buffer *respBuffer = new Buffer();

    respBuffer->add(aid);

    lastAID = aid;

    if (!shortRead)
    {
        if (cursor_pos < 4096) // 12 bit
        {
            respBuffer->add(0xC0|((cursor_pos>>6)&63));
            respBuffer->add(cursor_pos&63);
        }
        else if (cursor_pos < 16384) // 14 bit
        {
            respBuffer->add((cursor_pos>>8)&63);
            respBuffer->add(cursor_pos&0xFF);
        }
        else // 16 bit
        {
            respBuffer->add((cursor_pos>>8)&0xFF);
            respBuffer->add(cursor_pos&0xFF);
        }

        screen->getModifiedFields(respBuffer);

        respBuffer->dump();
    }

    if (aid == IBM3270_AID_CLEAR)
    {
        cursor_pos = 0;
        cursor_x = 0;
        cursor_y = 0;
        screen->setCursor(cursor_pos);
        screen->clear();
    }

    emit bufferReady(respBuffer);
}

void ProcessDataStream::interruptProcess()
{

    Buffer *b = new Buffer();

    b->add(IAC);
    b->add(IP);

    emit bufferReady(b);

}
