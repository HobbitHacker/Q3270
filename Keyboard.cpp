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

#include "Keyboard.h"
#include "Stores/KeyboardStore.h"
#include <QDebug>
#include <QDateTime>
#include "Q3270.h"


/**
 * @class   Keyboard Keyboard.h
 * @brief   Keyboard::Keyboard - Keyboard input processing
 *
 * @details The Keyboard class is how the keyboard is mapped to various 3270 and Q3270 functions. Each
 *          function has a name, and can be mapped to a key. There are 4 maps defined, one for normal, one
 *          for Shift, one for Ctrl and one for Alt (or Meta, depending). Keyboards can be mapped through
 *          the KeyboardTheme class to store user-defined maps. An internal Factory map which is set in here
 *          is the default and cannot be updated by the user.
 *
 *          Keyboard also handles type-ahead for when the host is still busy, but the user wants to continue
 *          typing to fill in the next field or enter the next command etc.
 *
 *          The Keyboard function map is held as a QMap with the name of the Qt key ("Enter", etc) and
 *          the target routine (Keyboard::enter, for example). If a key is pressed that doesn't generate
 *          a normal character, the function map is searched for a matching entry, and if found, called.
 */

const Keyboard::FunctionBinding Keyboard::bindings[] = {
    { "Enter",        &Keyboard::enter },
    { "Reset",        &Keyboard::reset },

    { "Up",           &Keyboard::cursorUp },
    { "Down",         &Keyboard::cursorDown },
    { "Left",         &Keyboard::cursorLeft },
    { "Right",        &Keyboard::cursorRight },

    { "Backspace",    &Keyboard::backspace },

    { "Tab",          &Keyboard::tab },
    { "Backtab",      &Keyboard::backtab },

    { "NewLine",      &Keyboard::newline },
    { "Home",         &Keyboard::home },
    { "EndLine",      &Keyboard::endline },

    { "EraseEOF",     &Keyboard::eraseEOF },

    { "Insert",       &Keyboard::insert },
    { "Delete",       &Keyboard::deleteKey },

    { "F1",           &Keyboard::fKey1 },
    { "F2",           &Keyboard::fKey2 },
    { "F3",           &Keyboard::fKey3 },
    { "F4",           &Keyboard::fKey4 },
    { "F5",           &Keyboard::fKey5 },
    { "F6",           &Keyboard::fKey6 },
    { "F7",           &Keyboard::fKey7 },
    { "F8",           &Keyboard::fKey8 },
    { "F9",           &Keyboard::fKey9 },
    { "F10",          &Keyboard::fKey10 },
    { "F11",          &Keyboard::fKey11 },
    { "F12",          &Keyboard::fKey12 },

    { "F13",          &Keyboard::fKey13 },
    { "F14",          &Keyboard::fKey14 },
    { "F15",          &Keyboard::fKey15 },
    { "F16",          &Keyboard::fKey16 },
    { "F17",          &Keyboard::fKey17 },
    { "F18",          &Keyboard::fKey18 },
    { "F19",          &Keyboard::fKey19 },
    { "F20",          &Keyboard::fKey20 },
    { "F21",          &Keyboard::fKey21 },
    { "F22",          &Keyboard::fKey22 },
    { "F23",          &Keyboard::fKey23 },
    { "F24",          &Keyboard::fKey24 },

    { "Attn",         &Keyboard::attn },

    { "PA1",          &Keyboard::paKey1 },
    { "PA2",          &Keyboard::paKey2 },
    { "PA3",          &Keyboard::paKey3 },

    { "Clear",        &Keyboard::clear },

    { "ToggleRuler",  &Keyboard::ruler },

    { "Copy",         &Keyboard::copy },
    { "Paste",        &Keyboard::paste },
    { "Info",         &Keyboard::info },
    { "Fields",       &Keyboard::fields },

    { "Blah",         &Keyboard::info }
};

QMap<QString, Keyboard::Handler> Keyboard::makeFunctionMap()
{
    QMap<QString, Handler> map;
    for (const auto &b : bindings)
        map.insert(b.name, b.method);
    return map;
}

Keyboard::Keyboard() : functionMap(makeFunctionMap())
{
    systemLock = false;
    insMode = false;

    bufferPos = 0;
    bufferEnd = 0;
    keyCount  = 0;
    waitRelease = false;

    clearBufferEntry();

    setMap(KeyboardMap::getFactoryMap());
}

QStringList Keyboard::allFunctionNames()
{
    QStringList list;

    for (const FunctionBinding &binding : bindings)
        list.append(QString::fromLatin1(binding.name));

    return list;
}

/**
 * @brief   Keyboard::clearBufferEntry - clear an entry in the keyboard buffer
 *
 * @details This routine clears the logical last entry in the buffer.
 */
void Keyboard::clearBufferEntry()
{
    kbBuffer[bufferEnd].modifiers = Qt::NoModifier;
    kbBuffer[bufferEnd].keyChar = QChar(00);
    kbBuffer[bufferEnd].mustMap = false;
    kbBuffer[bufferEnd].isMapped = false;
    kbBuffer[bufferEnd].key = 0;
}

/**
 * @brief   Keyboard::lockKeyboard - set the keyboard lock
 *
 * @details Set XSystem on the keyboard, and lock it, preventing further keystrokes being processed.
 */
void Keyboard::lockKeyboard()
{
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "Keyboard        : setEnterInhibit";
    systemLock = true;
    emit setEnterInhibit();
}

/**
 * @brief   Keyboard::unlockKeyboard - unlock the keyboard
 *
 * @details Remove XSystem and renable the keyboard, allowing keys to be processed. As this routine has unlocked
 *          the keyboard, immediately process any keys in the buffer.
 */
//void Keyboard::unlockKeyboard()
//{
//    systemLock = false;
//    nextKey();
//}

/**
 * @brief   Keyboard::eventFilter - process keyboard events
 * @param   dist
 * @param   event
 * @return  true if the event was processed, otherwise call the parent eventFilter.
 *
 * @details The eventFilter is used to handle keyboard events.  Depending on the key being
 *          pressed, the event can be processed immediately (as in the case of a normal character)
 *          or it may be deferred until the key is released (as in the case of, say, CTRL).
 *
 *          CTRL needs to be deferred because it may be used as the mapping for Enter, as well
 *          as being used for Copy (CTRL-C).
 */
bool Keyboard::eventFilter( QObject *dist, QEvent *event )
{

    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
    {
        return QObject::eventFilter(dist, event);
    }

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

//    printf("Keyboard        : Type: %s   Count: %d   Key: %d   Native: %d Modifiers: %d   nativeModifiers: %d   nativeVirtualKey: %d    text: %s (%2.2X)    Mapped:%d\n", event->type() == QEvent::KeyPress ? "Press" : "Release", keyEvent->count(), keyEvent->key(), keyEvent->nativeScanCode(), keyEvent->modifiers(), keyEvent->nativeModifiers(), keyEvent->nativeVirtualKey(), keyEvent->text().toLatin1().data(), keyEvent->text().toLatin1().data()[0], kbBuffer[bufferEnd].isMapped);
//    fflush(stdout);

    // If we need to wait for a key to be released (from a previous key press)
    // check to see if the event was a key press. If so, and it generated a character,
    // don't wait. This is to stop shifted letters waiting for their release, but enabling CTRL to be
    // mapped as a key (eg, RESET, ENTER) as well as allowing it to be used for a modified (eg CTRL-C).
    //
    // The 3270 display's ENTER key was 'typamatic' (ie, it repeated when held down) but Q3270 does
    // not support that behaviour for CTRL, SHIFT and the other modifier keys.

    bool keyUsed = false;
    if (keyEvent->type() == QEvent::KeyPress)
    {
        waitRelease = needtoWait(keyEvent);
        if (!waitRelease)
        {
            qDebug() << "Keyboard        : Processing Key - No need to wait";
            keyUsed = processKey();
        }

    }
    else if (waitRelease)
    {
        qDebug() << "Keyboard        : Processing key - KeyRelease";
        waitRelease = false;
        keyUsed = processKey();
    }
    else
    {
        qDebug() << "Keyboard        : Ignoring KeyRelease (already processed press)";
    }

    if (keyUsed)
    {
        return true;
    }

    return QObject::eventFilter(dist, event);

}

/**
 * @brief   Keyboard::processKey
 * @return  true if the key was processed, false if it is ignored
 *
 * @details processKey determines whether the current keyboard buffer position contains a key that
 *          doesn't need to be mapped (as in the case of a standard character key) or whether it is
 *          mapped to a Q3270 function, and whether they key must be mapped for it to make sense.
 *
 *          If the keyboard map contains a mapping for the key, the target function of the key
 *          is stored in the keyboard buffer, to be processed later by nextKey().
 *
 *          Eg, if the Home key is pressed, that must be mapped to a Q3270 function for it to be
 *          processed, but if it isn't mapped, the key is ignored.
 *
 *          The key is stored in the keyboard buffer, and if the key can be processed immediately,
 *          nextKey is called, but if the keyboard is locked, the current buffer position is incremented
 *          so that the next key can be stored.
 *
 *          The buffer is a wrap-around buffer.
 *
 *          If the key is processed or stored in the buffer, processKey returns true, otherwise
 *          it returns false.
 */
bool Keyboard::processKey()
{
    int key = kbBuffer[bufferEnd].key;

    kbBuffer[bufferEnd].isMapped = kbBuffer[bufferEnd].map->contains(key);

    //printf("Keyboard        : isSimpleText: %d - Key is mapped: %d\n", kbBuffer[bufferEnd].keyChar,kbBuffer[bufferEnd].isMapped);

    if (!kbBuffer[bufferEnd].isMapped)
    {
        if (!kbBuffer[bufferEnd].keyChar.isPrint())
        {
            kbBuffer[bufferEnd].mustMap = true;
        }
        key = kbBuffer[bufferEnd].nativeKey;
        kbBuffer[bufferEnd].isMapped = kbBuffer[bufferEnd].map->contains(key);
        qDebug() << "Keyboard        : Modifier key is mapped: " << kbBuffer[bufferEnd].isMapped;
    }

    keyCount++;
    qDebug() << "Keyboard        : Keycount incremented. Key:" << kbBuffer[bufferEnd].key << "isMapped:" << kbBuffer[bufferEnd].isMapped << "mustMap:" << kbBuffer[bufferEnd].mustMap;

    if ((kbBuffer[bufferEnd].key != 0  || kbBuffer[bufferEnd].isMapped) && connectedState)
    {

        // Store target mapping if key is mapped
        if (kbBuffer[bufferEnd].isMapped)
        {
            kbBuffer[bufferEnd].mapped = kbBuffer[bufferEnd].map->value(key).kbFunc;
        }

        if (kbBuffer[bufferEnd].mapped == &Keyboard::attn)
        {
            attn();   
        }
        else if (kbBuffer[bufferEnd].mapped == &Keyboard::reset)
        {
            reset();
        }
        else if (!systemLock)
        {
           if (kbBuffer[bufferEnd].mustMap && kbBuffer[bufferEnd].isMapped)
           {
               qDebug() << "Keyboard        : Processing nextKey (mapped and mustMap set)";
               nextKey();
           }
           else if (!kbBuffer[bufferEnd].mustMap)
           {
               qDebug() << "Keyboard        : Processing nextKey (not mapped, and mustMap not set)";
               nextKey();
           }
           else
           {
               qDebug() << "Keyboard        : Ignoring key";
               keyCount--;
               clearBufferEntry();
               return false;
           }
        }
        else
        {
            qDebug() << "Keyboard        : Locked, cannot process, buffering";

            bufferEnd++;

            if (bufferEnd > 1023) {
                bufferEnd = 0;
            }

            clearBufferEntry();
        }
    } else
    {
        qDebug() << "Keyboard        : Ignoring key";
        keyCount--;
        clearBufferEntry();
        return false;
    }

    return true;
}

/**
 * @brief   Keyboard::needtoWait - determine if Keyboard needs to wait for another keypress
 * @param   event - the keyboard event
 * @return  true if Keyboard needs to wait, false otherwise.
 *
 * @details needtoWait determines whether they keypress is one that might have another keypress needed
 *          to identify the keyboard mapping.
 *
 *          The modifier keys (CTRL, SHIFT etc) need another keypress (say SHIFT-A), so the SHIFT keypress
 *          is not something that can be processed. Note that a press and release of SHIFT could be
 *          mapped to a Q3270 function (though that would render much of the keyboard broken); a better
 *          example is the left and right CTRL keys being mapped to RESET and ENTER respectively.
 *
 *          needtoWait sets the keyboard buffer map to the appropriate one (eg, the Control map when
 *          a CTRL key is pressed).
 */
bool Keyboard::needtoWait(QKeyEvent *event)
{
    bool wait;

    switch(event->key())
    {
        case Qt::Key_Control:
        case Qt::Key_Shift:
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Meta:
//            printf("Keyboard        : Storing modifiers %8.8x\n", event->modifiers());
            kbBuffer[bufferEnd].modifiers = event->modifiers();
            kbBuffer[bufferEnd].nativeKey = event->nativeVirtualKey();
            qDebug() << "Keyboard        : Need to wait";
            wait = true;
            break;
        default:
//            printf("Keyboard        : Storing %d\n", event->key());
            kbBuffer[bufferEnd].modifiers = event->modifiers();
            kbBuffer[bufferEnd].key = event->key();
            if (event->text().length() > 0)
            {
                kbBuffer[bufferEnd].keyChar = event->text().at(0);
            }
            wait = false;
    }

    switch(kbBuffer[bufferEnd].modifiers)
    {
        case Qt::ControlModifier:
            qDebug() << "Keyboard        : Switching to CTRL map";
            kbBuffer[bufferEnd].map = &ctrlMap;
            kbBuffer[bufferEnd].mustMap = true;
            break;

        case Qt::AltModifier:
            qDebug() << "Keyboard        : Switching to ALT map";
            kbBuffer[bufferEnd].map = &altMap;
            kbBuffer[bufferEnd].mustMap = true;
            break;

        case Qt::ShiftModifier:
            qDebug() << "Keyboard        : Switching to SHIFT map";
            kbBuffer[bufferEnd].map = &shiftMap;
            kbBuffer[bufferEnd].mustMap = false;
            break;

        default:
            qDebug() << "Keyboard        : Switching to default map";
            kbBuffer[bufferEnd].mustMap = false;
            kbBuffer[bufferEnd].map = &defaultMap;
            break;
    }

    return wait;
}

/**
 * @brief   Keyboard::nextKey - process the next key in the buffer
 *
 * @details nextKey processes the next key in the buffer. The buffer position of the last key
 *          processed is stored in bufferPos, whereas the logical end of the buffer (ie, the end of the
 *          keys that are still to be processed) is stored in bufferEnd.
 *
 *          If the buffer position contains a mapped key, the routine to process that key is called,
 *          otherwise the key itself is processed.
 *
 *          If the keyboard buffer contains more keys to be processed, and the keyboard isn't locked
 *          recursively call nextKey().
 */
void Keyboard::nextKey()
{
    if (keyCount > 0)
    {
        qDebug() << "Keyboard        : Keycount now" << keyCount << "(buffer pos" << bufferPos << ", buffer end" << bufferEnd;
        keyCount--;
        if (kbBuffer[bufferPos].isMapped)
        {
            (this->*kbBuffer[bufferPos].mapped)();
        }
        else
        {
            // TODO: Might break
            emit key_Character(kbBuffer[bufferPos].keyChar.toLatin1(), insMode);
        }
        clearBufferEntry();
        if (bufferEnd != bufferPos)
        {
            bufferPos++;
            if (bufferPos > 1023)
            {
                bufferPos = 0;
            }
            if (!systemLock && keyCount > 0)
            {
                qDebug() << "Keyboard        : processing next key (more stored)";
                fflush(stdout);
                nextKey();
            }
        }
    }
}

/**
 * @brief   Keyboard::cursorUp - move the cursor up
 *
 * @details Move the cursor up one line
 */
void Keyboard::cursorUp()
{
    emit key_moveCursor(0, -1, Q3270_MOVE_CURSOR_RELATIVE);
}

/**
 * @brief   Keyboard::cursorDown - move the cursor down
 *
 * @details Move the cursor down one line
 */
void Keyboard::cursorDown()
{
    emit key_moveCursor(0, 1, Q3270_MOVE_CURSOR_RELATIVE);
}

/**
 * @brief   Keyboard::cursorUp - move the cursor right
 *
 * @details Move the cursor right one character
 */
void Keyboard::cursorRight()
{
    emit key_moveCursor(1, 0, Q3270_MOVE_CURSOR_RELATIVE);
}

/**
 * @brief   Keyboard::cursorLeft - move the cursor left
 *
 * @details Move the cursor left one character
 */
void Keyboard::cursorLeft()
{
    emit key_moveCursor(-1, 0, Q3270_MOVE_CURSOR_RELATIVE);
}

/**
 * @brief   Keyboard::backspace - process a backspace
 *
 * @details Perform a backspace function
 */
void Keyboard::backspace()
{
    emit key_Backspace();
}

/**
 * @brief   Keyboard::enter - perform the ENTER function
 *
 * @details Perform the 3270 ENTER function. Lock the keyboard, and call the AID routine. Switch
 *          Insert mode off.
 */
void Keyboard::enter()
{
    lockKeyboard();

    emit key_AID(IBM3270_AID_ENTER, Q3270_NOT_SHORT_READ);

    insMode = false;
    emit setInsert(false);
}

/**
 * @brief   Keyboard::tab - move the cursor to the next input field
 *
 * @details Move the cursor to the next input field after the current position.
 */
void Keyboard::tab()
{
    // Tab, starting at next character position
    emit key_Tab(1);
}

/**
 * @brief   Keyboard::backtab - move the cursor to the start of the previous input field
 *
 * @details Move the cursor to the start of the previous input field. This may be either the current
 *          field, if the cursor is not at the start of the field, or the previous field if the cursor
 *          is at the start of the field.
 */
void Keyboard::backtab()
{
    emit key_Backtab();
}

/**
 * @brief   Keyboard::home - move the cursor to the first input field
 *
 * @details Move the cursor to the first input field on the screen.
 */
void Keyboard::home()
{
    emit key_Home();
}

/**
 * @brief   Keyboard::eraseEOF - clear the rest of the field
 *
 * @details Set the rest of the field to nulls
 */
void Keyboard::eraseEOF()
{
    emit key_EraseEOF();
}

/**
 * @brief   Keyboard::insert - turn insert mode on
 *
 * @details Turn insert mode on and indicate this on the status bar
 */
void Keyboard::insert()
{
    insMode = !(insMode);

    if (insMode) {
        emit setInsert(true);
    } else {
        emit setInsert(false);
    }

}

/**
 * @brief   Keyboard::deleteKey - delete the next character
 *
 * @details Delete the character under the cursor and move the rest of the input field along
 */
void Keyboard::deleteKey()
{
    emit key_Delete();
}

/**
 * @brief   Keyboard::functionKey - process an F-key
 * @param   key - the key that was pressed
 *
 * @details functionKey is called when F1-F24 are pressed with the key number. Lock the keyboard,
 *          and switch insert mode off.
 */
void Keyboard::functionKey(int key)
{
    lockKeyboard();

    emit key_AID(key, Q3270_NOT_SHORT_READ);

    insMode = false;
    emit setInsert(false);
}

/**
 * @brief   Keyboard::fKey1 - F1
 *
 * @details Call functionKey with F1
 */
void Keyboard::fKey1()
{
    functionKey(IBM3270_AID_F1);
}

/**
 * @brief   Keyboard::fKey2 - F2
 *
 * @details Call functionKey with F2
 */
void Keyboard::fKey2()
{
    functionKey(IBM3270_AID_F2);
}

/**
 * @brief   Keyboard::fKey3 - F3
 *
 * @details Call functionKey with F3
 */
void Keyboard::fKey3()
{
    functionKey(IBM3270_AID_F3);
}

/**
 * @brief   Keyboard::fKey4 - F4
 *
 * @details Call functionKey with F4
 */
void Keyboard::fKey4()
{
    functionKey(IBM3270_AID_F4);
}

/**
 * @brief   Keyboard::fKey5 - F5
 *
 * @details Call functionKey with F5
 */
void Keyboard::fKey5()
{
    functionKey(IBM3270_AID_F5);
}

/**
 * @brief   Keyboard::fKey6 - F6
 *
 * @details Call functionKey with F6
 */
void Keyboard::fKey6()
{
    functionKey(IBM3270_AID_F6);
}

/**
 * @brief   Keyboard::fKey7 - F7
 *
 * @details Call functionKey with F7
 */
void Keyboard::fKey7()
{
    functionKey(IBM3270_AID_F7);
}

/**
 * @brief   Keyboard::fKey8 - F8
 *
 * @details Call functionKey with F8
 */
void Keyboard::fKey8()
{
    functionKey(IBM3270_AID_F8);
}

/**
 * @brief   Keyboard::fKey9 - F9
 *
 * @details Call functionKey with F9
 */
void Keyboard::fKey9()
{
    functionKey(IBM3270_AID_F9);
}

/**
 * @brief   Keyboard::fKey10 - F10
 *
 * @details Call functionKey with F10
 */
void Keyboard::fKey10()
{
    functionKey(IBM3270_AID_F10);
}

/**
 * @brief   Keyboard::fKey11 - F11
 *
 * @details Call functionKey with F11
 */
void Keyboard::fKey11()
{
    functionKey(IBM3270_AID_F11);
}

/**
 * @brief   Keyboard::fKey12 - F12
 *
 * @details Call functionKey with F12
 */
void Keyboard::fKey12()
{
    functionKey(IBM3270_AID_F12);
}

/**
 * @brief   Keyboard::fKey13 - F13
 *
 * @details Call functionKey with F13
 */
void Keyboard::fKey13()
{
    functionKey(IBM3270_AID_F13);
}

/**
 * @brief   Keyboard::fKey14 - F14
 *
 * @details Call functionKey with F14
 */
void Keyboard::fKey14()
{
    functionKey(IBM3270_AID_F14);
}

/**
 * @brief   Keyboard::fKey15 - F15
 *
 * @details Call functionKey with F15
 */
void Keyboard::fKey15()
{
    functionKey(IBM3270_AID_F15);
}

/**
 * @brief   Keyboard::fKey16 - F16
 *
 * @details Call functionKey with F16
 */
void Keyboard::fKey16()
{
    functionKey(IBM3270_AID_F16);
}

/**
 * @brief   Keyboard::fKey17 - F17
 *
 * @details Call functionKey with F17
 */
void Keyboard::fKey17()
{
    functionKey(IBM3270_AID_F17);
}

/**
 * @brief   Keyboard::fKey18 - F18
 *
 * @details Call functionKey with F18
 */
void Keyboard::fKey18()
{
    functionKey(IBM3270_AID_F18);
}

/**
 * @brief   Keyboard::fKey19 - F19
 *
 * @details Call functionKey with F19
 */
void Keyboard::fKey19()
{
    functionKey(IBM3270_AID_F19);
}

/**
 * @brief   Keyboard::fKey20 - F20
 *
 * @details Call functionKey with F20
 */
void Keyboard::fKey20()
{
    functionKey(IBM3270_AID_F20);
}

/**
 * @brief   Keyboard::fKey21 - F21
 *
 * @details Call functionKey with F21
 */
void Keyboard::fKey21()
{
    functionKey(IBM3270_AID_F21);
}

/**
 * @brief   Keyboard::fKey22 - F22
 *
 * @details Call functionKey with F22
 */
void Keyboard::fKey22()
{
    functionKey(IBM3270_AID_F22);
}

/**
 * @brief   Keyboard::fKey23 - F23
 *
 * @details Call functionKey with F23
 */
void Keyboard::fKey23()
{
    functionKey(IBM3270_AID_F23);
}

/**
 * @brief   Keyboard::fKey24 - F24
 *
 * @details Call functionKey with F24
 */
void Keyboard::fKey24()
{
    functionKey(IBM3270_AID_F24);
}

/**
 * @brief   Keyboard::attn - ATTN processing
 *
 * @details Attention processing - process ATTN and switch insert mode off
 */
void Keyboard::attn()
{
    emit key_Attn();

    insMode = false;
    emit setInsert(false);

    qDebug() << "Keyboard        : ATTN pressed";
}

/**
 * @brief   Keyboard::programaccessKey - process a PAx key
 * @param   aidKey - the PAx key
 *
 * @details programaccessKey is called by PA1-PA3 with the key number. Process the PA key and
 *          switch insert mode off.
 */
void Keyboard::programaccessKey(int aidKey)
{
    emit key_AID(aidKey, Q3270_SHORT_READ);

    insMode = false;
    emit setInsert(false);
}

/**
 * @brief   Keyboard::paKey1 - PA1
 *
 * @details Call programaccessKey with PA1
 */
void Keyboard::paKey1()
{
    programaccessKey(IBM3270_AID_PA1);
}

/**
 * @brief   Keyboard::paKey2 - PA2
 *
 * @details Call programaccessKey with PA2
 */
void Keyboard::paKey2()
{
    programaccessKey(IBM3270_AID_PA2);
}

/**
 * @brief   Keyboard::paKey3 - PA3
 *
 * @details Call programaccessKey with PA3
 */
void Keyboard::paKey3()
{
    programaccessKey(IBM3270_AID_PA3);
}

/**
 * @brief   Keyboard::newline - New Line processing
 *
 * @details Move the cursor to start of the next line and tab to the next input field
 */
void Keyboard::newline()
{
    emit key_Newline();
}

/**
 * @brief   Keyboard::reset - reset the keyboard
 *
 * @details Turn off insert mode and XSystem; update status bar accordingly.
 */
void Keyboard::reset()
{
    if (systemLock)
        return;

    insMode = false;
    systemLock = false;

    emit key_Reset();
    emit setInsert(false);
}

/**
 * @brief   Keyboard::endline - move the cursor to the last non-space in the field
 *
 * @details Move the cursor to the end of the current input field
 */
void Keyboard::endline()
{
    emit key_End();
}

/**
 * @brief   Keyboard::clear - Clear screen process
 *
 * @details Clear the screen
 */
void Keyboard::clear()
{
    emit key_AID(IBM3270_AID_CLEAR, Q3270_SHORT_READ);
}

/**
 * @brief   Keyboard::copy - copy the selection area to the clipboard
 *
 * @details Copy the selection area to the clipboard
 */
void Keyboard::copy()
{
    emit key_Copy();
}

/**
 * @brief   Keyboard::ruler - toggle the ruler
 *
 * @details Switch the ruler On or Off
 */
void Keyboard::ruler()
{
    emit key_toggleRuler();
}

/**
 * @brief   Keyboard::paste - paste from the clipboard
 *
 * @details Paste data to the screen from the clipboard
 */
void Keyboard::paste()
{
    clip = QApplication::clipboard();

    QString clipText = clip->text();
    int clipWidth = 0;

    for(int i = 0; i < clipText.length(); i++)
    {
        if (clipText.at(i) == '\n')
        {
            emit key_moveCursor(0 - clipWidth, 1, Q3270_MOVE_CURSOR_RELATIVE);
            clipWidth = 0;
        }
        else
        {
            emit key_Character(clipText.at(i).toLatin1(), insMode);
            clipWidth++;
        }
    }
}

/**
 * @brief   Keyboard::info - show details about the character under the cursor
 *
 * @details A debugging routine that will display all the information about the Cell under the cursor
 */
void Keyboard::info()
{
    emit key_showInfo();
}

/**
 * @brief   Keyboard::fields - show details about the fields defined on the screen
 *
 * @details A debugging routine that will display all the fields defined in the 3270 matrix.
 */
void Keyboard::fields()
{
    emit key_showFields();
}

/**
 * @brief   Keyboard::setMapping - set up a keyboard mapping
 * @param   key      - the key to be mapped
 * @param   function - the function to be called
 *
 * @details setMapping adds a keyboard mapping. It uses Qt-style keyboard notation (Shift+F1) and
 *          the Q3270 defined names of functions that can be called. The functions are defined in setMap.
 *
 *          functionMap contains a list of the available functions and unknown ones are ignored.
 *
 *          Recognised modifiers (CTRL, ALT) define which map the function is inserted into.
 *
 * @note    At the moment, multiple modifiers (CTRL-ALT-F1) are not supported.
 */
void Keyboard::setMapping(QString key, QString function)
{
    int keyCode;

    QMap<int, kbDets> *setMap = &defaultMap;

    // Decode "key" which should be something like:
    // Ctrl+A
    // Alt+1
    // We allow two special cases here, LCtrl and RCtrl as they do not exist in Qt.
    // After this code, keyCode will contain the key we want to store, and setMap will point
    // to the appropriate keyboard map.

    if (!key.compare("LCtrl", Qt::CaseInsensitive))
    {
        keyCode = Q3270_LEFT_CTRL;
        setMap = &ctrlMap;
    }
    else if (!key.compare("RCtrl", Qt::CaseInsensitive))
    {
        keyCode = Q3270_RIGHT_CTRL;
        setMap = &ctrlMap;
    } else
    {
        const auto keyList = key.split('+');

        // Extract last key of sequence (the actual key)
        const QKeySequence keys(keyList.constLast());
        keyCode = keys[0];

        // Extract any modifiers
        if (keyList.count() > 1)
        {
            // Build a string of modifiers - Alt+Ctrl
            QString keyMods;
            for(int j = 0; j < keyList.count() - 1; j++)
            {
                j == 0 ? keyMods = keyMods + keyList[j] :  keyMods = keyMods + "+" + keyList[j];
            }
            //TODO Ctrl+Alt type maps.
            if (!keyMods.compare("Alt", Qt::CaseInsensitive))
            {
                setMap = &altMap;
            }
            else if (!keyMods.compare("Ctrl", Qt::CaseInsensitive))
            {
                setMap = &ctrlMap;
            }
            else if (!keyMods.compare("Shift", Qt::CaseInsensitive))
            {
                setMap = &shiftMap;
            }
            else if (!keyMods.compare("Meta", Qt::CaseInsensitive))
            {
                setMap = &metaMap;
            }
        }
    }


    if (!functionMap.contains(function))
    {
        qDebug() << "Keyboard        : ERROR: Function" << function.toLatin1().data() << "unknown - ignored";
    }

    setMap->insert(keyCode, { functionMap.value(function), key, function });
}

/**
 * @brief   Keyboard::setMap - set a keyboard map
 * @param   kmap - the keyboard map
 *
 * @details setTheme takes a keyboard map and sets up the mappings of keys to functions.
 *          Keyboard maps are defined as QMap<QString, QStringList>; each function can have multiple
 *          keys assigned to it (e.g. F8 and PgDown are both defined to call F8 by default).
 */
void Keyboard::setMap(const KeyboardMap &kmap)
{
    // Clear the existing maps
    defaultMap.clear();
    ctrlMap.clear();
    altMap.clear();
    shiftMap.clear();
    metaMap.clear();

    // Iterate over each Mapping in the theme
    for (const Mapping &mapping : kmap.mappings) {
        // Each mapping may have multiple keys for the same function
        for (const QString &key : mapping.keys) {
            setMapping(key, mapping.functionName);
        }
    }
}

/**
 * @brief   Keyboard::setConnected - invoked when connection status changes
 * @param   state - true for connected, false for not
 *
 * @details setConnected is triggered when the connection is opened or closed so Keyboard knows
 *          when to process keys.
 */
void Keyboard::setConnected(bool state)
{
    connectedState = state;
}

void Keyboard::setLocked(const bool lock)
{
    systemLock = lock;
    if (!systemLock)
        nextKey();
}
