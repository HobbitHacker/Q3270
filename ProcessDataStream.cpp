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

ProcessDataStream::ProcessDataStream(TerminalView *t)
{
    terminal = t;

    primary_x = 0;
    primary_y = 0;
    primary_pos = 0;

    cursor_x = 0;
    cursor_y = 0;
    cursor_pos = 0;

    lastAID = IBM3270_AID_NOAID;
    lastWasCmd = false;

    setScreen();
}

void ProcessDataStream::setScreen(bool alternate)
{

    screen = terminal->setAlternateScreen(alternate);

    alternate_size = alternate;

    screen_x = screen->width();
    screen_y = screen->height();

    screenSize = screen_x * screen_y;
}

void ProcessDataStream::processStream(QByteArray &b, bool tn3270e)
{
    //FIXME: buffer size 0 shouldn't happen!
/*    if (b.isEmpty())
    {
        printf("\n\n[** DisplayDataStream: zero buffer size - returning **]\n\n");
        fflush(stdout);
        return;
    }
*/
//    b->dump();

    wsfProcessing = false;

    //TODO: Multiple structured field WRITE commands
    // Process Command codes
    // Structured Fields Require Multiple WRITE commands
    // Add logic to cycle round buffer for structure field length if appropriate so we can come
    // back here to restart.
    screen->resetCharAttr();

    buffer = b.begin();

    if (tn3270e)
    {
        unsigned char dataType = *buffer++;
        unsigned char requestFlag = *buffer++;
        unsigned char responseFlag = *buffer++;
        unsigned char seqNumber = ((uchar) *buffer++<<16);
        seqNumber+= (uchar) *buffer++;

        // TODO: Handling of TN3270E datatypes
        if (dataType != TN3270E_DATATYPE_3270_DATA)
        {
            printf("\n\n[** Unimplemented TN3270E command: %02X **]\n\n", dataType);
            return;
        }
    }

    // Process the incoming WRITE command
    switch((uchar) *buffer)
    {
        case IBM3270_EW:
        case IBM3270_CCW_EW:
            processEW(false);
            buffer++;
            break;
        case IBM3270_W:
        case IBM3270_CCW_W:
            processW();
            buffer++;
            break;
        case IBM3270_WSF:
        case IBM3270_CCW_WSF:
            wsfProcessing = true;
            buffer++;
            break;
        case IBM3270_EWA:
        case IBM3270_CCW_EWA:
            processEW(true);
            buffer++;
            break;
        case IBM3270_RM:
        case IBM3270_CCW_RM:
            processRM();
            buffer++;
            break;
        case IBM3270_RB:
        case IBM3270_CCW_RB:
            processRB();
            buffer++;
            break;
        default:
            printf("\n\n[** Unrecognised WRITE command: %02X - Block Ignored **]\n\n", (uchar) *buffer);
//            b->dump();
            processing = false;
            return;
    }

    while(buffer != b.end())
    {
        if (wsfProcessing)
        {
            processWSF();
        }
        else
        {
            processOrders();
        }
        buffer++;
    }

//    b.clear();
//    b->setProcessing(false);

    if (resetMDT)
    {

    }
    if (restoreKeyboard)
    {
        printf("[restore keyboard]");
        emit keyboardUnlocked();
    }
//    screen->dumpFields();
//    screen->dumpDisplay();
    fflush(stdout);

}

void ProcessDataStream::processOrders()
{
    switch((uchar) *buffer)
    {
        /* TODO: 3270 Order MF */
        case IBM3270_SF:
            processSF();
            break;
        case IBM3270_SFE:
            processSFE();
            break;
        case IBM3270_SBA:
            processSBA();
            break;
        case IBM3270_SA:
            processSA();
            break;
        case IBM3270_MF:
            printf("\n\n[** Unimplemented MF order **]\n\n");
            break;
        case IBM3270_IC:
            processIC();
            break;
        case IBM3270_PT:
            processPT();
            break;
        case IBM3270_RA:
            processRA();
            break;
        case IBM3270_EUA:
            processEUA();
            break;
        case IBM3270_GE:
            processGE();
            break;
        default:
            placeChar((uchar) *buffer);
    }
}

/*!
 *
 * \fn void processWCC()
 *
 * This processes the Write Control Character from the datastream, following WRITE or ERASE WRITE commands.
 *
 * The WCC character contains the following bits:
 *    Reset
 *    Reset MDT
 *    Restore Keyboard
 *    Alarm
 *
 */
void ProcessDataStream::processWCC()
{
    uchar wcc = *buffer;

    //TODO: Handle RESET
    int reset = (wcc>>6)&1;

    printf("[");

    resetMDT = wcc&1;
    restoreKeyboard  = (wcc>>1)&1;
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

    if (restoreKeyboard)
    {
        if(reset|resetMDT)
        {
            printf(",");
        }
        printf("restore keyboard");
        lastAID = IBM3270_AID_NOAID;
    }

    if (alarm)
    {
        if (restoreKeyboard|resetMDT|reset)
        {
            printf(",");
        }
        printf("alarm");
    }
    printf("]");

    lastWasCmd = true;
}


void ProcessDataStream::processW()
{
    printf("[Write ");

    buffer++;
    processWCC();

    printf("]");
    fflush(stdout);
}

void ProcessDataStream::processEW(bool alternate)
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

    buffer++;
    processWCC();

    printf("]");
    fflush(stdout);

    primary_x = 0;
    primary_y = 0;
    primary_pos = 0;

    cursor_x = 0;
    cursor_y = 0;
    cursor_pos = 0;

    screen->clear();

}


void ProcessDataStream::processRB()
{
    QByteArray screenContents;

    screenContents.append(lastAID);

    screen->addPosToBuffer(screenContents, cursor_pos);
    screen->getScreen(screenContents);

    emit bufferReady(screenContents);
}

void ProcessDataStream::processWSF()
{
    wsfProcessing = true;

    wsfLen = *buffer++<<8;
    wsfLen += *buffer++;

    // Decrease length by two-byte length field and structured field id byte
    wsfLen-=3;

    switch((uchar) *buffer)
    {
        case IBM3270_WSF_RESET:
            WSFreset();
            break;
        case IBM3270_WSF_READPARTITION:
            WSFreadPartition();
            break;
        case IBM3270_WSF_OB3270DS:
            WSFoutbound3270DS();
            break;
        default:
            printf("\n\n[** Unimplemented WSF command: %02X **]\n\n", *buffer);
            break;
    }
}

/**
 * @brief ProcessDataStream::processRM
 *        ReadModified - Extract modified fields from screen and return them.
 */
void ProcessDataStream::processRM()
{
    processAID(lastAID, false);
}

void ProcessDataStream::processSF()
{
    printf("[Start Field:");

    screen->setField(primary_pos, *++buffer, false);

    printf("]");
    fflush(stdout);

    incPos();

    lastWasCmd = true;
}

void ProcessDataStream::processSFE()
{
    uchar pairs = *++buffer;

    uchar type;
    uchar value;

    screen->resetExtended(primary_pos);

    for(int i = 1; i <= pairs; i++)
    {
        type = *++buffer;
        value = *++buffer;

        switch(type)
        {
            case IBM3270_EXT_3270:
                printf("[Field ");
                screen->setField(primary_pos, value, true);
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
                    case IBM3270_EXT_DEFAULT:
                        printf("[Default]");
                        screen->resetExtendedHilite(primary_pos);
                        break;
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
                        printf("\n\n[** Unimplemented SFE HILITE value - %2.2X-%2.2X - Ignored **]\n\n", type, value);
                        break;
                }
                break;
            default:
                printf("\n\n[** Unimplemented SFE attribute - %2.2X-%2.2X - Ignored **]\n\n", type, value);
                break;
        }
        fflush(stdout);
    }

    screen->setFieldAttrs(primary_pos);
    incPos();

    lastWasCmd = true;
}

void ProcessDataStream::processSBA()
{
    printf("[SetBufferAddress ");
    buffer++;
    int tmp_pos = extractBufferAddress();

    if (tmp_pos >= screenSize)
    {
        printf("[** SBA > screen size - discarded **]");
        return;
    }

    primary_pos = tmp_pos;

    primary_y = (primary_pos / screen_x);
    primary_x = primary_pos - (primary_y * screen_x);
    printf("%d,%d]", primary_x, primary_y);

    lastWasCmd = true;
}

void ProcessDataStream::processSA()
{
    uchar extendedType = *++buffer;
    uchar extendedValue = *++buffer;
    screen->setCharAttr(extendedType, extendedValue);

    lastWasCmd = true;
}

void ProcessDataStream::processMF()
{
    lastWasCmd = true;
}

void ProcessDataStream::processIC()
{
    printf("[InsertCursor(%d,%d)]", primary_x, primary_y);
    moveCursor(primary_x, primary_y, true);

    lastWasCmd = true;
}

/*!
 * \fn void processPT()
 *
 * Performs a Program Tab (PT) function.
 *
 * PT behvaves in different ways, depending on what the previous command/order was.
 *
 * The PT order advances the current buffer address to the address of the first
 * character position of the next unprotected field. If PT is issued when the current
 * buffer address is the location of a field attribute of an unprotected field, the buffer
 * advances to the next location of that field (one location). In addition, if PT does not
 * immediately follow a command, order, or order sequence (such as after the WCC,
 * IC, and RA respectively), nulls are inserted in the buffer from the current buffer
 * address to the end of the field, regardless of the value of bit 2
 * (protected/unprotected) of the field attribute for the field. When PT immediately
 * follows a command, order, or order sequence, the buffer is not modified.
 *
 * The PT order resets the character attribute to its default value for each character
 * set to nulls.
 *
 * The display stops its search for an unprotected field at the last location in the
 * character buffer. If a field attribute for an unprotected field is not found, the buffer
 * address is set to O. (If the display finds a field attribute for an unprotected field in
 * the last buffer location, the buffer address is also set to 0.)
 *
 * To continue the search for an unprotected field, a second PT order must be issued
 * immediately following the first one; in reply, the display begins its search at buffer
 * location O. If, as a result of a PT order, the display is still inserting nulls in each
 * character location when it terminates at the last buffer location, a new PT order
 * continues to insert nulls from buffer address 0 to the end of the current field.
 *
 */
void ProcessDataStream::processPT()
{
    //TODO: <PT><PT> is not catered for properly
    printf("[Program Tab]");
    buffer++;

    // If the current position is a field start and not protected, move one position
    if (screen->isFieldStart(primary_pos) && !screen->isProtected(primary_pos))
    {
        incPos();
        lastPTwrapped = false;
        return;
    }

    int nextField = screen->findNextUnprotectedField(primary_pos);

    // Skip field attribute byte, wrapping if need
    nextField = (nextField + 1) % screenSize;

    // If the field we found is before the current position, we stop at location 0
    if (nextField < primary_pos)
    {
        nextField = 0;
    }

    // If the last byte processed from the buffer was not a command, we clear the current field
    // - regardless of protection.

    if (!lastWasCmd)
    {
        int i;

        // Fill buffer with nulls until either the start of the next field or the end of the screen
        for(i = nextField; i < screenSize && !screen->isFieldStart(i); i++)
        {
            screen->setChar(i, IBM3270_CHAR_NULL, false, false);
        }

        // If we hit the end of the screen, record that
        if (i == screenSize - 1)
        {
            lastPTwrapped = true;
        }
    }
    else
    {
        lastPTwrapped = false;
    }

    // Adjust position according to the field we found
    primary_pos = nextField;
    primary_y = (primary_pos / screen_x);
    primary_x = primary_pos - (primary_y * screen_x);

    lastWasCmd = true;
}

void ProcessDataStream::processRA()
{
    buffer++;
    int endPos = extractBufferAddress();

    if (endPos > screenSize - 1)
    {
        endPos = screenSize - 1;
    }

    uchar newChar = *++buffer;

    bool geRA = false;

    // Check to see if it's <RA><GE><CHAR>
    if (newChar == IBM3270_GE)
    {
        geRA = true;
        newChar = *++buffer;
    }

    printf("[RepeatToAddress %d to %d (0x%2.2X)]", primary_pos, endPos, newChar);
    fflush(stdout);

    if (endPos <= primary_pos)
    {
        endPos += screenSize;
    }

    for(int i = primary_pos; i < endPos; i++)
    {
        int offset = i % screenSize;

        if (geRA)
        {
            screen->setGraphicEscape();
        }
        screen->setChar(offset, newChar, false, false);
    }

    primary_pos = endPos % screenSize;
    primary_y = (primary_pos / screen_x);
    primary_x = primary_pos - (primary_y * screen_x);

    lastWasCmd = true;
}

void ProcessDataStream::processEUA()
{
    printf("[EraseUnprotected to Address ");

    buffer++;
    int stopAddress = extractBufferAddress();
    printf("]");

    screen->eraseUnprotected(primary_pos, stopAddress);
    restoreKeyboard = true;

    lastWasCmd = true;
}

void ProcessDataStream::processGE()
{
    printf("[GraphicEscape ");
    screen->setGraphicEscape();
    placeChar((uchar) *++buffer);
    printf("]");
    fflush(stdout);

    lastWasCmd = false;
}

void ProcessDataStream::WSFoutbound3270DS()
{
    printf("[Outbound 3270DS");
    uchar partition = *++buffer;

    printf(" partition #%d ", partition);

    uchar cmnd = *++buffer;

    wsfLen-=2;

    switch(cmnd)
    {
        case IBM3270_W:
            printf("Write ");
            buffer++;
            processWCC();
            wsfLen--;
            break;
        default:
            printf("\n\n [** Unimplemented WSF command %02X - Ignored **]\n\n", cmnd);
    }
    while(wsfLen>0)
    {
        processOrders();
        wsfLen--;
    }
}

void ProcessDataStream::WSFreset()
{
    printf("\n\n[** Reset Partition %02X - Not Implemented **]\n\n", *++buffer);
    fflush(stdout);
    return;
}

void ProcessDataStream::WSFreadPartition()
{
    uchar partition = *++buffer;
    uchar type = *++buffer;

    printf("ReadPartition %d - type %2.2X\n", partition, type);

    QByteArray queryReply;

    queryReply.append((uchar) IBM3270_AID_SF);

    replySummary(queryReply);

    emit bufferReady(queryReply);
}

void ProcessDataStream::replySummary(QByteArray &queryReply)
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
       82 - 10000010
         1  ALT         graphic escape
         0  MULTID   no multiple LCIDs
         0  LOADABLE no LOAD PS
         0  EXT      no LOAD PS EXTENDED
         0  MS       no more than one char size only
         0  CH2      no DBCS
         1  GF          CGCSID present
         0  - reserved -
       00 - no LOAD PS slot size required
       09 - default width
       0c - default height
       00 00 00 00 - LOAD PS format types supported (none)
       07 - length of each descriptor

          00 - SET  - char set
          10 - Flags
              00010000
                 0 - LOAD    Non-loadable
                 0 - TRIPLE  Not triple plane
                 0 - CHAR    Single-byte character set
                 1 - CB      No LCID compare
                 0000 - reserved
          00 LCID - Local character set id
          [ bytes SW and SH missing as MS is zero ]
          [ bytes SUBSN and SUBSN missing as CH2 is zero ]

          CGCSID - present as GF set to one
0080      02 b9 - character set number - 697
          00 25 - code page - 037

          01 - char set
          10 - No LCID compare
          f1 - Local character set id
          03 c3 - character set 963
          01 36 - code page 310

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
                            0x00, 0x09,    /* Length */
                            IBM3270_SF_QUERYREPLY,
                            IBM3270_SF_QUERYREPLY_SUMMARY,
                            IBM3270_SF_QUERYREPLY_COLOUR,
                            IBM3270_SF_QUERYREPLY_IMPPARTS,
                            IBM3270_SF_QUERYREPLY_USABLE,
                            IBM3270_SF_QUERYREPLY_CHARSETS,
                            IBM3270_SF_QUERYREPLY_HIGHLIGHT,
//                            IBM3270_SF_QUERYREPLY_PARTS
                   };

    unsigned char qrcolour[] = {
                                 0x00, 0x16,
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
                              0x00, 0x11,
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

    unsigned char qhighlight[] = {
                                    0x00, 0x0D,
                                    IBM3270_SF_QUERYREPLY,
                                    IBM3270_SF_QUERYREPLY_HIGHLIGHT,
                                    0x04,  /* Number of pairs */
                                    0x00, 0xF0,  /* Default */
                                    0xF1, 0xF1,  /* Blink */
                                    0xF2, 0xF2,  /* Reverse */
                                    0xF4, 0xF4   /* Uscore */
                                 };

    unsigned char qusablearea[] = {
                                    0x00, 0x19,
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

    unsigned char qcharsets[] = {
                                0x00, 0x1B,
                                IBM3270_SF_QUERYREPLY,
                                IBM3270_SF_QUERYREPLY_CHARSETS,
                                0x82,            /* GE, CGCSGID supported only - 10000010 */
                                                 /* x....... - ALT */
                                                 /*            0 - Graphic Escape not supported */
                                                 /*            1 - Graphic Escape not supported */
                                                 /* .x...... - MULTID */
                                                 /*            0 - Multiple LCIDs are not supported */
                                                 /*            1 - Multiple LCIDs are supported */
                                                 /* ..x..... - LOADABLE */
                                                 /*            0 - LOAD PS are not supported */
                                                 /*            1 - LOAD PS are supported */
                                                 /* ...x.... - EXT */
                                                 /*            0 - LOAD PS EXTENDED is not supported */
                                                 /*            1 - LOAD PS EXTENDED is supported */
                                                 /* ....x... - MS */
                                                 /*            0 - Only one character slot size is supported */
                                                 /*            1 - More than one size of character slot is supported */
                                                 /* .....x.. - CH2 */
                                                 /*            0 - 2-byte coded character sets are not supported */
                                                 /*            1 - 2 byte coded character sets are supported */
                                                 /* ......x. - GF */
                                                 /*            0 - CGCSGID is not present */
                                                 /*            1 - CGCSGID is present */
                                                 /* .......x - reserved */

                                0x00,            /* x.xxxxxx - reserved */
                                                 /*  x       - Programmed Symbols Character Slot */
                                                 /*             0 = Load PS slot size must match */
                                                 /*             1 = Load PS slot size match not required */
                                0x04, 0x03,      /* Default character slot width, height */
                                0x00, 0x00, 0x00, 0x00,   /* LOAD PS format types supported - none */
                                    0x07,   /* length of descriptor areas   */

                                      // Descriptor set 1
                                      0x00, /* char set 0 */
                                      0x10, /* flags: no LCID compare */
                                      0x00, /* local character set id */
                                      0x02, 0xB9, /* character set number */
                                      0x00, 0x25,  /* code page 037 */

                                      // Descriptor set 2
                                      0x01, /* char set 1 */
                                      0x10, /* flags: no LCID compare */
                                      0xF1, /* local character set id */
                                      0x03, 0xC3, /* character set number */
                                      0x01, 0x36  /* code page 310 */
    };

    unsigned char qalphaparts [] = {
                    0x00, 0x08,   /* Length */
                    0x81,         /* Query Reply */
                    0x84,         /* Alphanumeric partitions */
                    0x02,         /* 2 partitions supported */
                    0xFF, 0xFE,   /* Storage allowed */
                    0x00          /* Partition flags: */
                                  /* x.......   SCROLL - Scrollable */
                                  /* .xx.....   RES    - Reserved */
                                  /* ...x....   AP     - All points addressable */
                                  /* ....x...   PROT   - Partition Protection */
                                  /* .....x..   COPY   - Presentation space local copy supported */
                                  /* ......x.   MODIFY - Modify partition supported */
                                  /* .......x   RES    - Reserved */
    };

    qpart[14] = terminal->alternate->width();
    qpart[16] = terminal->alternate->height();

    qusablearea[6] = 0x00;
    qusablearea[7] = qpart[14];

    qusablearea[8] = 0x00;
    qusablearea[9] = qpart[16];

    QSizeF screenSizeMM = QGuiApplication::primaryScreen()->physicalSize();
    QSizeF screenSizePix = QGuiApplication::primaryScreen()->size();

    int xmm = screenSizeMM.width();
    int ymm = screenSizeMM.height();

    qusablearea[11]  = (xmm & 0xFF00) >> 8;
    qusablearea[12] = (xmm & 0xFF);

    qusablearea[15] = (ymm & 0xFF00) >> 8;
    qusablearea[16] = (ymm & 0xFF);

    int x = screenSizePix.width();
    int y = screenSizePix.height();

    qusablearea[13] = (x & 0xFF00) >> 8;
    qusablearea[14] = (x & 0xFF);

    qusablearea[17] = (y & 0xFF00) >> 8;
    qusablearea[18] = (y & 0xFF);

    qusablearea[19] = ((int)terminal->primary->gridWidth() & 0xFF00) >> 8;
    qusablearea[20] = ((int)terminal->primary->gridWidth() & 0xFF);

    qusablearea[21] = ((int)terminal->primary->gridHeight() & 0xFF00) >> 8;
    qusablearea[22] = ((int)terminal->primary->gridHeight() & 0xFF);

    qusablearea[23] = ((qusablearea[5] * qusablearea[7]) & 0xFF00) >> 8;
    qusablearea[24] = ((qusablearea[5] * qusablearea[7]) & 0xFF);


    addBytes(queryReply, qrt, 9);
    addBytes(queryReply, qrcolour, 22);
    addBytes(queryReply, qpart, 17);
    addBytes(queryReply, qhighlight, 13);
    addBytes(queryReply, qusablearea, 25);
    addBytes(queryReply, qcharsets, 27);
//    buffer->addBlock(qalphaparts, 8);
}

void ProcessDataStream::addBytes(QByteArray &b, uchar *s, int l)
{
    for (int i = 0; i < l; i++)
    {
        b.append(s[i]);

        //Double up 0xFF bytes
        if (s[i] == 0xFF)
        {
            b.append(0xFF);
        }
    }
}

int ProcessDataStream::extractBufferAddress()
{
    //TODO: non-12/14 bit addresses & EBCDIC characters

    uchar sba1 = *buffer;
    uchar sba2 = *++buffer;

    switch((sba1>>6)&3)
    {
        case 0b00:     // 14 bit
//            printf("%2.2X%2.2X (%d%d - 14 bit)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return ((sba1&63)<<8)+sba2;
        case 0b01:     // 12 bit
//            printf("%2.2X%2.2X (%d%d - 12 bit)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return ((sba1&63)<<6)+(sba2&63);
        case 0b11:     // 12 bit
//            printf("%2.2X%2.2X (%d%d - 12 bit)", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return ((sba1&63)<<6)+(sba2&63);
        default: // case 0b10: - reserved
            printf("\n\n[** Unimplemented BufferAddress - %2.2X%2.2X - bitmask %d%d **]\n\n", sba1, sba2, (sba1>>7)&1, (sba1>>6)&1);
            return 0;
    }
}

void ProcessDataStream::placeChar()
{
    uchar ebcdic = *buffer;
    placeChar(ebcdic);
}

void ProcessDataStream::placeChar(uchar ebcdic)
{
    switch(ebcdic)
    {
        case IBM3270_CHAR_NULL:
            screen->setChar(primary_pos, 0x00, false, false);
            break;
        default:
            screen->setChar(primary_pos, ebcdic, false, false);
    }

    incPos();

    lastWasCmd = false;
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
        if (screen->isAskip(cursor_pos))
        {
            tab();
        }
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

    screen->setCursor(cursor_pos);
    screen->drawRuler(cursor_x, cursor_y);

    emit cursorMoved(cursor_x, cursor_y);
}

void ProcessDataStream::backspace()
{
    // If we're at a protected field, do nothing
    if (screen->isProtected(cursor_pos))
        return;

    // Discover whether the previous cursor position is a field start
    int tempCursorPos = cursor_pos == 0 ? screenSize : cursor_pos - 1;

    if (screen->isFieldStart(tempCursorPos))
        return;

    // Backspace one character
    moveCursor(-1 , 0);
}

void ProcessDataStream::tab(int offset)
{
    int nf = screen->findNextUnprotectedField(cursor_pos + offset);

    cursor_y = (nf / screen_x);
    cursor_x = nf - (cursor_y * screen_x);
//    printf("Unprotected field found at %d (%d,%d) ", nf, cursor_x, cursor_y);

    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);
}

void ProcessDataStream::backtab()
{
    int pf = screen->findPrevUnprotectedField(cursor_pos);

    cursor_y = (pf / screen_x);
    cursor_x = pf - (cursor_y * screen_x);
//    printf("Backtab: Unprotected field found at %d (%d,%d) ", pf, cursor_x, cursor_y);

    // Move cursor right (skip attribute byte)
    moveCursor(1, 0);

}

void ProcessDataStream::home()
{
    // Find first field on screen; this might be position 0, so we need to look starting at the last screen pos
    int nf = screen->findNextUnprotectedField(screenSize - 1);
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

void ProcessDataStream::endline()
{
    if (screen->isProtected(cursor_pos))
    {
        return;
    }

    int endPos = cursor_pos + screenSize;

    QString field;
    int endField;

    int i = cursor_pos;
    int offset = cursor_pos;

    endField = cursor_pos;
    bool letter = false;

    while(i < endPos && !screen->isProtected(offset) && !screen->isFieldStart(offset))
    {
//        qDebug() << "Offset: " << offset << " Protected: " << screen->isProtected(offset) << " Character: " << (uchar)screen->getChar(offset) << "Field Start:" << screen->isFieldStart(offset);
        uchar thisChar = screen->getChar(offset);
        if (letter && (thisChar == 0x00 || thisChar == ' '))
        {
            endField = offset;
            letter = false;
        }

        if (thisChar != 0x00 && thisChar != ' ')
        {
            letter = true;

        }

        field.append(screen->getChar(offset));
        offset = ++i % screenSize;
    }

    cursor_pos = endField;

    cursor_x = (cursor_pos / screen_x);
    cursor_x = cursor_pos - (cursor_y * screen_x);

    moveCursor(cursor_x, cursor_y, true);

}

void ProcessDataStream::toggleRuler()
{
    screen->toggleRuler();
    screen->drawRuler(cursor_x, cursor_y);
}

void ProcessDataStream::processAID(int aid, bool shortRead)
{
    QByteArray respBuffer = QByteArray();

    respBuffer.append(aid);

    lastAID = aid;

    if (!shortRead)
    {
        screen->addPosToBuffer(respBuffer, cursor_pos);
        screen->getModifiedFields(respBuffer);
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
    QByteArray b = QByteArray();

    b.append((uchar) IAC);
    b.append((uchar) IP);

    emit bufferReady(b);
}
