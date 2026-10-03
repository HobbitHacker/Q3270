/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "Q3270.h"
#include "ProcessDataStream.h"
#include "Terminal.h"

// TODO: PROGxxx codes
// https://bitsavers.trailing-edge.com/pdf/ibm/pc/3270/3270_PC/SC23-0103-2_3270_PC_Control_Program_Users_Guide_and_Reference_Jul84.pdf
// B-57 onwards

/**
 * @brief   ProcessDataStream::ProcessDataStream - 3270 Data Stream processing
 * @param   t - the terminal object
 *
 * @details Initialise the ProcessDataStream object, setting power on settings.
 */
ProcessDataStream::ProcessDataStream(Terminal *t, DisplayScreen *screen, FileTransfer &f)
    : terminal(t)
    , screen(screen)
    , fileTransfer(f)
{
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

    emit setAlternateScreen(alternate);

    alternate_size = alternate;

    screen_x = terminal->terminalWidth(alternate);
    screen_y = terminal->terminalHeight(alternate);

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

//    qApp->processEvents();
    wsfProcessing = false;
    lastwasWrite = false;

    reply.clear();

    //TODO: Multiple structured field WRITE commands
    // Process Command codes
    // Structured Fields Require Multiple WRITE commands
    // Add logic to cycle round buffer for structure field length if appropriate so we can come
    // back here to restart.
    screen->resetCharAttr();

    buffer = b.begin();

    if (tn3270e)
    {
        quint8 dataType = nextBufferByte();
        quint8 requestFlag = nextBufferByte();
        quint8 responseFlag = nextBufferByte();
        quint16 seqNumber = nextBufferHalfWord();

        // TODO: Handling of TN3270E datatypes
        if (dataType != TN3270E_DATATYPE_3270_DATA)
        {
            printf("\n\n[** Unimplemented TN3270E command: %02X **]\n\n", dataType);
            return;
        }
    }

    // Process the incoming command
    switch(nextBufferByte())
    {
        case IBM3270_EW:
        case IBM3270_CCW_EW:
            processEW(false);
            break;
        case IBM3270_W:
        case IBM3270_CCW_W:
            processW();
            break;
        case IBM3270_WSF:
        case IBM3270_CCW_WSF:
            wsfProcessing = true;
            break;
        case IBM3270_EWA:
        case IBM3270_CCW_EWA:
            processEW(true);
            break;
        case IBM3270_RM:
        case IBM3270_CCW_RM:
            processRM();
            break;
        case IBM3270_RMA:
        case IBM3270_CCW_RMA:
            processRMA();
            break;
        case IBM3270_EAU:
        case IBM3270_CCW_EAU:
            processEAU();
            break;
        case IBM3270_RB:
        case IBM3270_CCW_RB:
            processRB();
            break;
        default:
            printf("\n\n[** Unrecognised WRITE command: %02X - Block Ignored **]\n\n", (uchar) *buffer);
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
    }


    // qDebug() << QDateTime::currentMSecsSinceEpoch() << "ProcessDataStream: processingComplete";

    if (lastwasWrite)
        emit processingComplete();

    if (resetMDT)
    {

    }

    if (restoreKeyboard)
    {
    // qDebug() << QDateTime::currentMSecsSinceEpoch() << "ProcessDataStream: unlockKeyboard";
        emit unlockKeyboard();
    }

    // Reset character attributes
    screen->resetCharAttr();

    // Check for active file transfer
    if (lastwasWrite && fileTransfer.isExpected()) {
        fileTransfer.checkCUT(screen, reply);
    }

    // If a response was built, send it
    if (!reply.isEmpty())
    {
        // qDebug() << QDateTime::currentMSecsSinceEpoch() << "ProcessDataStream: Sending Query reply";
        emit bufferReady(reply);
    }

}

/**
 * @brief   ProcessDataStream::processOrders - Process 3270 Orders
 *
 * @details After the 3270 Command in the data stream, there may be 3270 Orders that need processing.
 */
void ProcessDataStream::processOrders()
{
    quint8 order = nextBufferByte();

    switch(order)
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
            placeChar(order);
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
    uchar wcc = nextBufferByte();

    qDebug() << QString("ProcessDataStream: WCC = %1").arg(wcc, 2, 16);

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
    lastwasWrite = true;
}

/**
 * @brief   ProcessDataStream::processW - The 3270 WRITE command
 *
 * @details Process the WRITE command, which is used to change the currently displayed screen.
 */
void ProcessDataStream::processW()
{
    printf("[Write ");

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

    screen->getScreen(reply);
}

/**
 * @brief   ProcessDataStream::processEAU - The 3270 ERASE ALL UNPROTECTED command
 *
 * @details The EAU (not to be confused with EUA!) erases all unprotected fields on the screen,
 *          resets MDTs positions the cursor to the first enterable field on the screen, and
 *          resets the AID.
 */
void ProcessDataStream::processEAU()
{
    qDebug() << "[EraseAllUnprotected]";

    screen->eraseUnprotected(0, screenSize, Q3270::EraseResetMDT::ResetMDT);

    restoreKeyboard = true;

    screen->setCursor(screen->findNextUnprotectedField(0));
    lastAID = IBM3270_AID_NOAID;
}

/**
 * @brief   ProcessDataStream::processWSF - The 3270 WRITE STRUCTURED FIELD command
 *
 * @details The WSF command is used to perform many types of extended operation, such as allowing multiple
 *          partitions, printing, loading of programmed symbols and more.
 */
void ProcessDataStream::processWSF()
{
    QByteArray::Iterator wsfBuffer = buffer;

    wsfLen = nextBufferHalfWord();

    qDebug() << QString("Structured Field - length %1, type %2").arg(wsfLen, 4, 16).arg((uchar)*buffer, 2, 16);

    switch(nextBufferByte())
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
        case IBM3270_WSF_TRANSFER:
            WSFtransfer();
            break;
        default:
            qDebug() << QString("ProcessDataStream: [** Unimplemented WSF command: %1 - ignored **]").arg((quint8)*buffer, 2, 16);
            break;
    }

    // The buffer is advanced outside of the switch statement to ensure that it is always moved past the WSF data,
    // regardless of whether the command was implemented or not, and whether the bytes in the buffer were all
    // consumed or left in the buffer having been extracted in other ways.
    buffer = wsfBuffer + wsfLen;
}

/**
 * @brief   ProcessDataStream::processRM - The 3270 READ MODIFIED command
 *
 * @details Extract modified fields from screen and return them to the host.
 */
void ProcessDataStream::processRM()
{
    qDebug() << "[ReadModified]";

    if (lastAID == IBM3270_AID_SF)
    {
        fileTransfer.readModified(reply);
    }
    else if (fileTransfer.getMode() == FileTransfer::FTMode::CUT)
    {
        fileTransfer.cutRetransmit(reply);
    }
    else
    {
        screen->processAID(lastAID, Q3270_SHORT_READ);
    }
}

/**
 * @brief   ProcessDataStream::processRMA - The 3270 READ MODIFIED ALL command
 * 
 * @details Extract all modified fields from screen and return them to the host.
 * 
 * @note    Read Modified All Command
 *          The Read Modified All command operates exactly like the Read Modified
 *          command, except that it always causes both the addresses and the data from all
 *          modified fields to be sent to the application program independent of the AID code.
 *          This includes those AID codes that would normally result in a short read, as well
 *          as for trigger action (X' 7F' ), Clear Partition (X '6A' ), and Read Partition (X' 61 ').
 */
void ProcessDataStream::processRMA()
{
    screen->processAID(lastAID, Q3270_NOT_SHORT_READ);
}

/**
 * @brief   ProcessDataStream::processSF - The 3270 START FIELD order
 *
 * @details SF starts a field on the 3270 display.
 */
void ProcessDataStream::processSF()
{
    uchar fa = nextBufferByte();
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

    processAttributePairs(IBM3270_SFE);

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
    uchar extendedType = nextBufferByte();
    uchar extendedValue = nextBufferByte();
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
    processAttributePairs(IBM3270_MF);

    if (screen->isFieldStart(primary_pos)) {
        screen->cascadeAttrs(primary_pos);
    }

    lastWasCmd = true;
}

/**
 * @brief   ProcessDataStream::handleAttributePairs - Handle attribute pairs in SFE and MF
 * @param   numPairs - the number of attribute pairs
 * @param   mode     - IBM3270_SFE or IBM3270_MF
 *
 * @details This function processes the attribute pairs in both SFE and MF orders. If mode is SFE, it
 *          ensures that a field is defined for the SFE order; if one is not present, it creates a
 *          field with the default attributes. This is done because the SFE order can be used to set
 *          attributes for a field that is not yet defined.
 *  
 *          If mode is MF, it checks that the current position is a field start. If not, it rejects the
 *          MF order and returns. This is because the MF order is used to modify an existing field's
 *          attributes, and it cannot be used to create a new field.
 */
void ProcessDataStream::processAttributePairs(int mode)
{
    quint8 numPairs = nextBufferByte();

    bool fieldDefined = false;

    for (int i = 0; i < numPairs; i++)
    {
//        if (buffer + 2 > QByteArray().end()) {
//            printf("[** Insufficient data for attribute pairs **]\n");
//            return;
//        }
        quint8 type = nextBufferByte();
        quint8 value = nextBufferByte();

        // Handle field attribute (X'C0') separately
        if (type == IBM3270_EXT_3270) {
            if (mode == IBM3270_SFE) {
                // Start Field Extended: Define a new field at the current buffer position
                screen->setField(primary_pos, value, true);
                fieldDefined = true; // Mark that a field attribute was supplied
            }
        }
        else if (mode == IBM3270_MF && !screen->isFieldStart(primary_pos)) {
            // Modify Field: Reject processing if no field exists at the current position
            printf("[** MF order rejected—No field at buffer position **]\n");
            return;
        }
        else {
            // Process remaining attributes (colors, highlighting, etc.)
            switch (type)
            {
                case IBM3270_EXT_FG_COLOUR:
                    screen->setExtendedColour(primary_pos, true, value); // Set foreground color
                    break;
                case IBM3270_EXT_BG_COLOUR:
                    screen->setExtendedColour(primary_pos, false, value); // Set background color
                    break;
                case IBM3270_EXT_HILITE:
                    switch (value)
                    {
                        case IBM3270_EXT_DEFAULT:
                        case IBM3270_EXT_HI_NORMAL:
                            screen->resetExtendedHilite(primary_pos); // Reset highlight to default
                            break;
                        case IBM3270_EXT_HI_BLINK:
                            screen->setExtendedBlink(primary_pos); // Apply blink effect
                            break;
                        case IBM3270_EXT_HI_REVERSE:
                            screen->setExtendedReverse(primary_pos); // Apply reverse effect
                            break;
                        case IBM3270_EXT_HI_USCORE:
                            screen->setExtendedUscore(primary_pos); // Apply underscore effect
                            break;
                        default:
                            // Log unsupported highlight attributes for visibility
                            printf("[** Unimplemented attribute - %2.2X-%2.2X - Ignored **]\n", type, value);
                            break;
                    }
                    break;
                default:
                    // Log unsupported attributes for debugging
                    printf("[** Unimplemented attribute - %2.2X-%2.2X - Ignored **]\n", type, value);
                    break;
            }
        }
    }

    // If no field was explicitly set in SFE, apply a default field attribute
    if (mode == IBM3270_SFE && !fieldDefined) {
        screen->setField(primary_pos, 0x00, true);
    }
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
    int endPos = extractBufferAddress();

    uchar newChar = nextBufferByte();

    bool geRA = false;
    if (newChar == IBM3270_GE)
    {
        geRA = true;
        newChar = nextBufferByte();
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
    int stopAddress = extractBufferAddress();

    if (stopAddress >= screenSize || stopAddress < 0)
    {
        printf("[*** EUA buffer end position invalid %d - discarded ***]", stopAddress);
        return;
    }

    printf("[EraseUnprotected to Address %d]", stopAddress);

    screen->eraseUnprotected(primary_pos, stopAddress, Q3270::EraseResetMDT::DoNotResetMDT);

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
    placeChar(nextBufferByte());

    lastWasCmd = false;
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

    uchar sba1 = nextBufferByte();
    uchar sba2 = nextBufferByte();

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

/**
 * @brief   ProcessDataStream::nextBufferByte - extract the next byte from the buffer
 * @return  the next byte from the buffer
 *
 * @details This function is used to extract a single byte from the data stream buffer. The buffer is
 *          advanced by one byte.
 */
quint8 ProcessDataStream::nextBufferByte()
{
    return static_cast<quint8>(*buffer++);
}

/**
 * @brief   ProcessDataStream::nextBufferHalfWord - extract the next two bytes from the buffer as a 16 bit value
 * @return  the next two bytes from the buffer as a 16 bit value
 *
 * @details This function is used to extract two bytes from the data stream buffer and combine them into
 *          a single 16 bit value. The buffer is advanced by two bytes.
 */
quint16 ProcessDataStream::nextBufferHalfWord()
{
    quint16 hi = static_cast<quint8>(*buffer++);
    quint16 lo = static_cast<quint8>(*buffer++);
    return (hi << 8) | lo;
}
