/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "Q3270.h"
#include "ProcessDataStream.h"
#include "Terminal.h"

/**
 * @brief   ProcessDataStream::ProcessDataStream - 3270 Data Stream processing
 * @param   t - the terminal object
 *
 * @details Initialise the ProcessDataStream object, setting power on settings.
 */
ProcessDataStream::ProcessDataStream(Terminal *t)
{
    terminal = t;

    primary_pos = 0;

    lastAID = IBM3270_AID_NOAID;
    lastWasCmd = false;

    setScreen();
}

/**
 * @brief   ProcessDataStream::setScreen - change the terminal to display primary or alternate screen
 * @param   alternate - true for alternate, false for primary
 *
 * @details Called when the 3270 Data Stream includes an EW or EWA command. The size of the screen
 *          is adjusted accordingly.
 */
void ProcessDataStream::setScreen(bool alternate)
{

    screen = terminal->setAlternateScreen(alternate);

    alternate_size = alternate;

    screen_x = screen->width();
    screen_y = screen->height();

    screenSize = screen_x * screen_y;
}

/**
 * @brief   ProcessDataStream::processStream - Process an incoming 3270 Data Stream
 * @param   b       - the bytes in the 3270 data stream
 * @param   tn3270e - true for TN3270-E processing, false otherwise
 *
 * @details Called when the incoming 3270 Data Stream is complete. Commands and orders are processed.
 */
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

    qApp->processEvents();
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

    while(buffer < b.end())
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
//        QCoreApplication::processEvents();
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
//    fflush(stdout);

    screen->refresh();

}

/**
 * @brief   ProcessDataStream::processOrders - Process 3270 Orders
 *
 * @details After the 3270 Command in the data stream, there may be 3270 Orders that need processing.
 */
void ProcessDataStream::processOrders()
{
    switch((uchar) *buffer)
    {
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
            processMF();
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

/**
 * @brief   ProcessDataStream::processWCC - process the Write Control Character after a WRITE type command
 *
 * @details This processes the Write Control Character from the datastream, following WRITE or ERASE WRITE
 *          commands.
 *
 *          The WCC character contains the following bits (extracted from the 3270 Data Stream manual):
 *
 *          Bit | Function
 *          --- | --------
 *            0 | If the Reset function is not supported, the only function of bits 0 and 1 is to make the
 *            ^ | WCC byte an EBCDIC/ASCII-translatable character.
 *            ^ |
 *            ^ | If the Reset function is supported, bit 1 controls reset/no reset and bit 0 has no
 *            ^ | function. When bit 1 is used for the Reset function the WCC byte is no longer always
 *            ^ | EBCDIC/ASCII-translatable.
 *            1 | WCC reset bit. When set to 1, it resets partition characteristics to their system-defined
 *            ^ | defaults. When set to 0, the current characteristics remain unchanged (no reset operations
 *            ^ | are performed).
 *          2,3 | For Printers*
 *            4 | Start Printer. When set to 1, it initiates a local copy operation of the display surface
 *            ^ | at the completion of the write operation.
 *            5 | Sound-alarm bit. When set to 1, it sounds the audible alarm at the end of the operation
 *            ^ | if that device has an audible alarm.
 *            6 | Keyboard restore bit. When set to 1, this bit unlocks the keyboard. It also resets the
 *            ^ | AID byte.
 *            7 | Bit 7 resets MDT bits in the field attributes. When set to 1, all MDT bits in the device's
 *            ^ | existing character buffer are reset before any data is written or orders are performed.
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
        screen->resetMDTs();
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

    screen->resetCharAttr();

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processW - The 3270 WRITE command
 *
 * @details Process the WRITE command, which is used to change the currently displayed screen.
 */
void ProcessDataStream::processW()
{
    printf("[Write ");

    buffer++;
    processWCC();

    printf("]");
//    fflush(stdout);
}

/**
 * @brief   ProcessDataStream::processEW - The 3270 ERASE WRITE and ERASE WRITE ALTERNATE commands
 * @param   alternate - true for EWA, false for EW
 *
 * @details EW and EWA are used to clear the screen and rebuild the display from scratch. Depending on
 *          the currently displayed screen (primary or alternate) the screen may change size to the
 *          opposite (from EW to EWA or from EWA to EW). This is the only time the size can change.
 */
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
//    fflush(stdout);

    primary_pos = 0;

    screen->clear();

}

/**
 * @brief   ProcessDataStream::processRB - The 3270 READ BUFFER command
 *
 * @details The RB command is used to extract the entire content of the display and return it to the
 *          host.
 */
void ProcessDataStream::processRB()
{
    printf("[ReadBuffer]");

    QByteArray screenContents = QByteArray();

    screen->getScreen(screenContents);

    emit bufferReady(screenContents);
}

/**
 * @brief   ProcessDataStream::processWSF - The 3270 WRITE STRUCTURED FIELD command
 *
 * @details The WSF command is used to perform many types of extended operation, such as allowing multiple
 *          partitions, printing, loading of programmed symbols and more.
 */
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
 * @brief   ProcessDataStream::processRM - The 3270 READ MODIFIED command
 *
 * @details Extract modified fields from screen and return them to the host.
 */
void ProcessDataStream::processRM()
{
    printf("[ReadModified]");
    screen->processAID(lastAID, false);
}

/**
 * @brief   ProcessDataStream::processSF - The 3270 START FIELD order
 *
 * @details SF starts a field on the 3270 display.
 */
void ProcessDataStream::processSF()
{
    unsigned char fa = *++buffer;
//    printf("[Start Field: %02X ", fa);

    screen->setField(primary_pos, fa, false);
//    screen->setFieldAttrs(primary_pos);

//    printf("]");
//    fflush(stdout);

    incPos();

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processSFE - The 3270 START FIELD EXTENDED order
 *
 * @details SFE starts an extended field on the 3270 display, allowing more colours and attributes
 *          than the basic SF order.
 */
void ProcessDataStream::processSFE()
{
    screen->resetExtended(primary_pos);
//    screen->setFieldAttrs(primary_pos);

    bool blink;
    bool reverse;
    bool uscore;
    bool reset;
    bool fattr;
    bool fgset;
    bool bgset;

    int foreground;
    int background;

    int field;

    uchar pairs = *++buffer;

    uchar type;
    uchar value;

    field = 0;

    blink = false;
    reverse = false;
    uscore = false;
    reset = false;
    fattr = false;
    fgset = false;
    bgset = false;

    foreground = 0;
    background = 0;
//    printf("[StartFieldExtended ");

    for(int i = 1; i <= pairs; i++)
    {
            type = *++buffer;
            value = *++buffer;

            switch(type)
            {
            case IBM3270_EXT_3270:
                field = value;
                fattr = true;
                break;
            case IBM3270_EXT_FG_COLOUR:
//                printf("[Extended FG %02X]", value);
                fgset = true;
                foreground = value;
                break;
            case IBM3270_EXT_BG_COLOUR:
//                printf("Extended BG %02dX]", value);
                bgset = true;
                background = value;
                break;
            case IBM3270_EXT_HILITE:
                switch(value)
                {
                    case IBM3270_EXT_DEFAULT:
//                        printf("[Highlight Default]");
                        reset = true;
                        break;
                    case IBM3270_EXT_HI_NORMAL:
//                        printf("[Reset Extended]");
                        reset = true;
                        break;
                    case IBM3270_EXT_HI_BLINK:
//                        printf("[Blink]");
                        blink = true;
                        break;
                    case IBM3270_EXT_HI_REVERSE:
//                        printf("[Reverse]");
                        reverse = true;
                        break;
                    case IBM3270_EXT_HI_USCORE:
//                        printf("[Underscore]");
                        uscore = true;
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
    }


    if (fattr) {
        screen->setField(primary_pos, field, true);
    } else {
        screen->setField(primary_pos, 0x00, true);
    }

    if (blink) {
        screen->setExtendedBlink(primary_pos);
    }

    if (reverse) {
        screen->setExtendedReverse(primary_pos);
    }

    if (uscore) {
        screen->setExtendedUscore(primary_pos);
    }

    if (reset) {
        screen->resetExtendedHilite(primary_pos);
    }

    if (fgset) {
        screen->setExtendedColour(primary_pos, true, foreground);
    }

    if (bgset) {
        screen->setExtendedColour(primary_pos, false, background);
    }

//    printf("]");
//    fflush(stdout);

    lastWasCmd = true;

    incPos();

}

/**
 * @brief   ProcessDataStream::processSBA - The 3270 SET BUFFER ADDRESS order
 *
 * @details The SBA order moves the position of the next character to be written.
 */
void ProcessDataStream::processSBA()
{
//    printf("[SetBufferAddress ");
    buffer++;
    int tmp_pos = extractBufferAddress();

    if (tmp_pos >= screenSize || tmp_pos < 0)
    {
        printf("[** SBA invalid address %d - discarded **]", tmp_pos);
        return;
    }

    primary_pos = tmp_pos;

/*    int primary_y = (primary_pos / screen_x);
    int primary_x = primary_pos - (primary_y * screen_x);
    printf("%d,%d]", primary_x, primary_y);
*/
    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processSA - The 3270 SET ATTRIBUTE order
 *
 * @details The SBA order set character attributes that will be used until the next SA, write-type command,
 *          or CLEAR key is pressed.
 */
void ProcessDataStream::processSA()
{
    uchar extendedType = *++buffer;
    uchar extendedValue = *++buffer;
//    printf("[SetAttribute %02X,%02X]", extendedType, extendedValue);
    screen->setCharAttr(extendedType, extendedValue);

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processMF - The 3270 MODIFY FIELD order
 *
 * @details The MF order changes an existing field's attributes and extended attributes.
 *
 * @note    Some of this processing is almost identical to that in SFE.
 */
void ProcessDataStream::processMF()
{
    unsigned char count = *++buffer;
    printf("[[ModifyField Pairs=%02X]", count);
    for (int i = 0; i < count; i++)
    {
        unsigned char type = *++buffer;
        unsigned char value = *++buffer;
        if (screen->isFieldStart(primary_pos)) {
            printf("[Pair %02X][Attribute %02X Value %02X]", count, type, value);

            switch(type)
            {
                case IBM3270_EXT_3270:
                    screen->setField(primary_pos, value, true);
                    break;
                case IBM3270_EXT_FG_COLOUR:
                    printf("[Extended FG %02X]", value);
                    screen->setExtendedColour(primary_pos, true, value);
                    break;
                case IBM3270_EXT_BG_COLOUR:
                    printf("Extended BG %02dX]", value);
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
                            printf("[Blink]");
                            screen->setExtendedBlink(primary_pos);
                            break;
                        case IBM3270_EXT_HI_REVERSE:
                            printf("[Reverse]");
                            screen->setExtendedReverse(primary_pos);
                            break;
                        case IBM3270_EXT_HI_USCORE:
                            printf("[Blink]");
                            screen->setExtendedUscore(primary_pos);
                            break;
                        default:
                            printf("\n\n[** Unimplemented MF HILITE value - %2.2X-%2.2X - Ignored **]\n\n", type, value);
                            break;
                    }
                    break;
                default:
                    printf("\n\n[** Unimplemented MF attribute - %2.2X-%2.2X - Ignored **]\n\n", type, value);
                    break;
            }

        }
    }

    if (screen->isFieldStart(primary_pos)) {
        screen->cascadeAttrs(primary_pos);
    }

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processIC - The 3270 INSERT CURSOR order
 *
 * @details The IC order is used to position the cursor on the screen (unrelated to the position
 *          of the next character to be written by the data stream).
 */
void ProcessDataStream::processIC()
{
    int primary_y = (primary_pos / screen_x);
    int primary_x = primary_pos - (primary_y * screen_x);
    printf("[InsertCursor(%d,%d)]", primary_x, primary_y);

    screen->setCursor(primary_x, primary_y);

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processPT() - The 3270 PROGRAM TAB order
 *
 * @details The PT order performs a few convoluted functions, depending on what the previous command
 *          or order was.
 *
 *          The PT order advances the current buffer address to the address of the first character position
 *          of the next unprotected field. If PT is issued when the current buffer address is the location
 *          of a field attribute of an unprotected field, the buffer advances to the next location of that
 *          field (one location). In addition, if PT does not immediately follow a command, order, or order
 *          sequence (such as after the WCC, IC, and RA respectively), nulls are inserted in the buffer from
 *          the current buffer address to the end of the field, regardless of the value of bit 2 (protected/
 *          unprotected) of the field attribute for the field. When PT immediately follows a command, order,
 *          or order sequence, the buffer is not modified.
 *
 *          The PT order resets the character attribute to its default value for each character set to nulls.
 *
 *          The display stops its search for an unprotected field at the last location in the character
 *          buffer. If a field attribute for an unprotected field is not found, the buffer address is set to
 *          0. (If the display finds a field attribute for an unprotected field in the last buffer location,
 *          the buffer address is also set to 0.)
 *
 *          To continue the search for an unprotected field, a second PT order must be issued immediately
 *          following the first one; in reply, the display begins its search at buffer location 0. If, as a
 *          result of a PT order, the display is still inserting nulls in each character location when it
 *          terminates at the last buffer location, a new PT order continues to insert nulls from buffer
 *          address 0 to the end of the current field.
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
            screen->setChar(i, IBM3270_CHAR_NULL, false);
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

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processRA - The 3270 REPEAT TO ADDRESS order
 *
 * @details The RA order repeats the specified character from the current address to the specified one.
 */
void ProcessDataStream::processRA()
{
    buffer++;
    int endPos = extractBufferAddress();

    uchar newChar = *++buffer;

    bool geRA = false;
    if (newChar == IBM3270_GE)
    {
        geRA = true;
        newChar = *++buffer;
    }

    if (endPos > screenSize || endPos < 0)
    {
        printf("[*** RA buffer end position invalid %d - discarded ***]", endPos);
        return;
    }

    if (endPos <= primary_pos)
    {
        endPos += screenSize;
    }

    for(int i = primary_pos; i < endPos; i++)
    {
        if (geRA)
        {
            screen->setGraphicEscape();
        }
        placeChar(newChar);
    }

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processEUA - The 3270 ERASE UNPROTECTED TO ADDRESS order
 *
 * @details The EUA (not to be confused with EAU!) clears all unprotected fields on the screen until the
 *          specified address.
 */
void ProcessDataStream::processEUA()
{

    buffer++;
    int stopAddress = extractBufferAddress();

    if (stopAddress >= screenSize || stopAddress < 0)
    {
        printf("[*** EUA buffer end position invalid %d - discarded ***]", stopAddress);
        return;
    }

    printf("[EraseUnprotected to Address %d]", stopAddress);

    screen->eraseUnprotected(primary_pos, stopAddress);

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::processGE - The 3270 GRAPHIC ESCAPE order
 *
 * @details The GE order writes a graphic escape character (that is, one from codepage 310) to the screen.
 */
void ProcessDataStream::processGE()
{
    screen->setGraphicEscape();
    placeChar((uchar) *++buffer);

    lastWasCmd = false;
}

/**
 * @brief   ProcessDataStream::WSFoutbound3270DS - The 3270 Structured Field Outbound 3270DS
 *
 * @details The Outbound 3270DS is used to direct output to a specific partition. There are
 *          four possible incoming (outbound from the host) operations:
 *
 *          @li Write
 *          @li Erase/Write
 *          @li Erase/Write Alternate
 *          @li Erase All Unprotected
 *
 *          These operations can be directed to a specific partition after the Create Partition
 *          structured field has been used.
 *
 * @note    Q3270 supports only WRITE currently.
 */
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

/**
 * @brief   ProcessDataStream::WSFreset - The 3270 Structured Field Reset
 *
 * @details The Reset is used to reset the terminal to its power on state.
 */
void ProcessDataStream::WSFreset()
{
    printf("\n\n[** Reset Partition %02X - Not Implemented **]\n\n", *++buffer);
    return;
}

/**
 * @brief   ProcessDataStream::WSFreadPartition - The 3270 Structured Field Read Partition
 *
 * @details The Read Partition function allows the host to read a specific partition or to
 *          query the device to report functions that it supports.
 *
 * @note    Q3270 only supports the Query function.
 */
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

/**
 * @brief    ProcessDataStream::replySummary - The 3270 Structured Field Query Reply (Summary)
 * @param    queryReply - the buffer to add the reply to
 *
 * @details  The Query Reply (Summary) structured field is used to inform the host of a summary of
 *           the functions that the device supports.
 *
 *           Here, the reply summary also includes, as structured fields following the summary, the
 *           device capabililties.
 *
 *           It is this function that allows the host the utilise the dynamic screen sizes.
 */
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
                                    0x04,        /* Number of pairs */
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

    qpart[14] = terminal->terminalWidth(true);
    qpart[16] = terminal->terminalHeight(true);

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

    qusablearea[19] = ((int)terminal->gridWidth(false) & 0xFF00) >> 8;
    qusablearea[20] = ((int)terminal->gridWidth(false) & 0xFF);

    qusablearea[21] = ((int)terminal->gridHeight(false) & 0xFF00) >> 8;
    qusablearea[22] = ((int)terminal->gridHeight(false) & 0xFF);

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

/**
 * @brief   ProcessDataStream::addBytes - add bytes to a buffer
 * @param   b - the buffer
 * @param   s - the data
 * @param   l - the length
 *
 * @details addBytes is used to add bytes to a buffer that will be sent to the host. Any 0xFF characters
 *          are doubled up.
 */
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

/**
 * @brief   ProcessDataStream::extractBufferAddress - extract two bytes from the buffer as a screen address
 * @return  the screen position
 *
 * @details The incoming 3270 data stream contains buffer addresses (screen positions) that are encoded
 *          in different ways depending on the first two bits of the first byte. (12 or 14 bits).
 *
 *          It is possible to use 16 bit, but only once a partition has been created explicitly.
 *
 *          Bit 0 and 1 patterns:
 *
 *          Setting | Address
 *          ------- | ----------
 *            00    | 14 bit. xxyyyyyy yyyyyyyy -> 00yyyyyy yyyyyyyy
 *            01    | 12 bit. xxyyyyyy xxyyyyyy -> 0000yyyy yyyyyyyy
 *            10    | Unsupported. Reserved.
 *            11    | 12 bit. xxyyyyyy xxyyyyyy -> 0000yyyy yyyyyyyy
 *
 *          16 bit addressing, if used, would use all 16 bits.
 */
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

/**
 * @brief   ProcessDataStream::placeChar - place a character onto the screen
 * @param   ebcdic - the EBCDIC character to be placed.
 *
 * @details This routine is used to put a character from the data stream onto the screen.
 */
void ProcessDataStream::placeChar(uchar ebcdic)
{
    screen->setChar(primary_pos, ebcdic, false);
    incPos();
    lastWasCmd = false;
}

/**
 * @brief   ProcessDataStream::incPos - increment the screen buffer position
 *
 * @details The screen buffer position is incremented by various commands and orders. The screen buffer
 *          starts at 0 for the top-left position, and continues sequentially until the bottom-right,
 *          wrapping from the end of one line to the beginning of the next line down.
 */
void ProcessDataStream::incPos()
{
    if (++primary_pos >= screenSize)
    {
        primary_pos = 0;
    }
}
