#include "keyboard.h"

Keyboard::Keyboard(DisplayDataStream *d, SocketConnection *c)
{    
    display = d;
    socket = c;

    setFactoryMaps();

    //TODO: Call DisplayDataStream methods directly?
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Up, &Keyboard::cursorUp));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Down, &Keyboard::cursorDown));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Right, &Keyboard::cursorRight));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Left, &Keyboard::cursorLeft));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Backspace, &Keyboard::cursorLeft));

    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Enter, &Keyboard::enter));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Tab, &Keyboard::tab));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Backtab, &Keyboard::backtab));

    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Home, &Keyboard::home));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Return, &Keyboard::newline));

    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_End, &Keyboard::eraseEOF));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Insert, &Keyboard::insert));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Delete, &Keyboard::deleteKey));

    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F1, &Keyboard::fKey1));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F2, &Keyboard::fKey2));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F3, &Keyboard::fKey3));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F4, &Keyboard::fKey4));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F5, &Keyboard::fKey5));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F6, &Keyboard::fKey6));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F7, &Keyboard::fKey7));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F8, &Keyboard::fKey8));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F9, &Keyboard::fKey9));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F10, &Keyboard::fKey10));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F11, &Keyboard::fKey11));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F12, &Keyboard::fKey12));

    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Escape, &Keyboard::attn));

    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_PageUp, &Keyboard::fKey7));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_PageDown, &Keyboard::fKey8));

    ctrlMap.insert(std::pair<int, doSomething>(Q3270_LEFT_CTRL, &Keyboard::reset));
    ctrlMap.insert(std::pair<int, doSomething>(Q3270_RIGHT_CTRL, &Keyboard::enter));

    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F1, &Keyboard::fKey13));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F2, &Keyboard::fKey14));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F3, &Keyboard::fKey15));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F4, &Keyboard::fKey16));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F5, &Keyboard::fKey17));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F6, &Keyboard::fKey18));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F7, &Keyboard::fKey19));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F8, &Keyboard::fKey20));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F9, &Keyboard::fKey21));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F10, &Keyboard::fKey22));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F11, &Keyboard::fKey23));
    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_F12, &Keyboard::fKey24));

    shiftMap.insert(std::pair<int, doSomething>(Qt::Key_Backtab, &Keyboard::backtab));

    altMap.insert(std::pair<int, doSomething>(Qt::Key_1, &Keyboard::paKey1));
    altMap.insert(std::pair<int, doSomething>(Qt::Key_2, &Keyboard::paKey2));
    altMap.insert(std::pair<int, doSomething>(Qt::Key_3, &Keyboard::paKey3));

    lock = false;
    insMode = false;

    bufferPos = 0;
    bufferEnd = 0;
    keyCount  = 0;
    waitRelease = false;

    clearBufferEntry();

    connect(d, &DisplayDataStream::keyboardUnlocked, this, &Keyboard::unlockKeyboard);
    printf("Keyboard unlocked\n");
    fflush(stdout);
}

void Keyboard::clearBufferEntry()
{
    kbBuffer[bufferEnd].modifiers = Qt::NoModifier;
    kbBuffer[bufferEnd].keyChar = "";
    kbBuffer[bufferEnd].mustMap = false;
    kbBuffer[bufferEnd].isMapped = false;
    kbBuffer[bufferEnd].key = 0;

}

void Keyboard::lockKeyboard()
{
    lock = true;
    printf("Keyboard locked\n");
    fflush(stdout);
    emit setLock(Indicators::SystemLock);
}

void Keyboard::unlockKeyboard()
{
    lock = false;
    printf("Keyboard unlocked\n");
    fflush(stdout);
    emit setLock(Indicators::Unlocked);
    nextKey();
}

bool Keyboard::eventFilter( QObject *dist, QEvent *event )
{

    if (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease)
    {
        return QObject::eventFilter(dist, event);
    }

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    printf("Keyboard: Type: %s   Count: %d   Key: %d   Native: %d Modifiers: %d   nativeModifiers: %d   nativeVirtualKey: %d    text: %s (%2.2X)    Mapped:%d\n", event->type() == QEvent::KeyPress ? "Press" : "Release", keyEvent->count(), keyEvent->key(), keyEvent->nativeScanCode(), keyEvent->modifiers(), keyEvent->nativeModifiers(), keyEvent->nativeVirtualKey(), keyEvent->text().toLatin1().data(), keyEvent->text().toLatin1().data()[0], kbBuffer[bufferEnd].isMapped);
    fflush(stdout);

    // If we need to wait for a key to be released (from a previous key press)
    // check to see if the event was a key press. If so, and it generated a character,
    // don't wait. This is to stop shifted letters waiting for their release, but allowing
    // an F-key to wait for its press.

    bool keyUsed = false;

    if (keyEvent->type() == QEvent::KeyPress)
    {
        waitRelease = needtoWait(keyEvent);
    }

    if (waitRelease)
    {
        printf("Waiting for release\n");
        fflush(stdout);
        if  (event->type() == QEvent::KeyPress && kbBuffer[bufferEnd].key != 0)
        {
            printf("Processing key - KeyPress or key stored\n");
            fflush(stdout);
            waitRelease = false;
            keyUsed = processKey(keyEvent);
        } else if (event->type() == QEvent::KeyRelease)
        {
            printf("Processing key - KeyRelease\n");
            fflush(stdout);
            waitRelease = false;
            keyUsed = processKey(keyEvent);
        } else
        {
            printf("Not a KeyPress or no key stored\n");
            fflush(stdout);
        }
    }
    else if (event->type() == QEvent::KeyPress)
    {
        printf("KeyPress\n");
        fflush(stdout);
        if (kbBuffer[bufferEnd].key == 0)
        {
            printf("Set Waiting for release - no key stored\n");
            fflush(stdout);
            waitRelease = true;
            keyUsed = true;
        }
        else
        {
            printf("Processing Key - key stored or no need to wait\n");
            fflush(stdout);
            keyUsed = processKey(keyEvent);
        }

    }
    if (keyUsed)
    {
        return true;
    }

    return QObject::eventFilter(dist, event);

}

bool Keyboard::processKey(QKeyEvent *keyEvent)
{

    printf("Searching for %d in map %ld\n", kbBuffer[bufferEnd].key, kbBuffer[bufferEnd].map);
    fflush(stdout);

    std::unordered_map<int, doSomething>::const_iterator got;

    kbBuffer[bufferEnd].isMapped = false;

    got = kbBuffer[bufferEnd].map->find(kbBuffer[bufferEnd].key);

    kbBuffer[bufferEnd].isMapped = (got != kbBuffer[bufferEnd].map->end());

    printf("Mapped: %d\n", kbBuffer[bufferEnd].isMapped);

    if (!kbBuffer[bufferEnd].isMapped)
    {
        if (kbBuffer[bufferEnd].keyChar == "")
        {
            kbBuffer[bufferEnd].mustMap = true;
        }
        got = kbBuffer[bufferEnd].map->find(kbBuffer[bufferEnd].nativeKey);
        kbBuffer[bufferEnd].isMapped = (got != kbBuffer[bufferEnd].map->end());
        printf(" - Modifier Mapped: %d\n", kbBuffer[bufferEnd].isMapped);
    }
    fflush(stdout);

    if (kbBuffer[bufferEnd].key != 0  || kbBuffer[bufferEnd].isMapped)
    {

        // Store target mapping if key is mapped
        if (kbBuffer[bufferEnd].isMapped) {
            kbBuffer[bufferEnd].mapped = got->second;
        }

        keyCount++;

        if (kbBuffer[bufferEnd].mapped == &Keyboard::attn)
        {
            attn();
        }
        else if (kbBuffer[bufferEnd].mapped == &Keyboard::reset)
        {
            reset();
        }
        else if (!lock)
        {
           if (kbBuffer[bufferEnd].mustMap && kbBuffer[bufferEnd].isMapped)
           {
               nextKey();
           }
           else if (!kbBuffer[bufferEnd].mustMap)
           {
               nextKey();
           }
           else
           {
               printf("Dropping key");
               fflush(stdout);
               clearBufferEntry();
               return false;
           }
        }
        else
        {
            printf("Keyboard: locked, cannot process, buffering\n");
            fflush(stdout);
        }

        bufferEnd++;

        if (bufferEnd > 1023) {
            bufferEnd = 0;
        }

        clearBufferEntry();
    }

    return true;
}



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
            printf("Storing modifiers %8.8x\n", event->modifiers());
            kbBuffer[bufferEnd].modifiers = event->modifiers();
            kbBuffer[bufferEnd].nativeKey = event->nativeVirtualKey();
            wait = true;
            break;
        default:
            printf("Storing %d\n", event->key());
            kbBuffer[bufferEnd].modifiers = event->modifiers();
            kbBuffer[bufferEnd].key = event->key();
            kbBuffer[bufferEnd].keyChar = event->text();
            wait = false;
    }
    fflush(stdout);

    switch(kbBuffer[bufferEnd].modifiers)
    {
        case Qt::ControlModifier:
            printf("Switching to CTRL map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].map = &ctrlMap;
            kbBuffer[bufferEnd].mustMap = true;
            break;

        case Qt::AltModifier:
            printf("Switching to ALT map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].map = &altMap;
            kbBuffer[bufferEnd].mustMap = true;
            break;

        case Qt::ShiftModifier:
            printf("Switching to SHIFT map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].map = &shiftMap;
            break;

        default:
            printf("Switching to default map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].map = &defaultMap;
            break;
    }

    return wait;
}

void Keyboard::nextKey()
{
    if (keyCount > 0)
    {
        if (kbBuffer[bufferPos].isMapped)
        {
            (this->*kbBuffer[bufferPos].mapped)();
        }
        else
        {
            // TODO: Might break
            display->insertChar(kbBuffer[bufferPos].keyChar.toUtf8()[0], insMode);
        }
        bufferPos++;
        if (bufferPos > 1023)
        {
            bufferPos = 0;
        }
        keyCount--;
        if (!lock && keyCount > 0)
        {
            nextKey();
        }
    }
}

void Keyboard::cursorUp()
{
    display->moveCursor(0, -1);
}

void Keyboard::cursorDown()
{
    display->moveCursor(0, 1);
}

void Keyboard::cursorRight()
{
    display->moveCursor(1, 0);
}

void Keyboard::cursorLeft()
{
    display->moveCursor(-1, 0);
}

void Keyboard::enter()
{
    lockKeyboard();
    Buffer *b = display->processFields(IBM3270_AID_ENTER);
    socket->sendResponse(b);

    insMode = false;
}

void Keyboard::tab()
{
    display->tab();
}

void Keyboard::backtab()
{
    display->backtab();
}

void Keyboard::home()
{
    display->home();
}

void Keyboard::eraseEOF()
{
    display->eraseField();
}

void Keyboard::insert()
{
    insMode = !(insMode);

    if (insMode) {
        emit setLock(Indicators::InsertMode);
    } else {
        emit setLock(Indicators::OvertypeMode);
    }

}

void Keyboard::deleteKey()
{
    display->deleteChar();
}

void Keyboard::functionKey(int key)
{
    lockKeyboard();
    Buffer *b = display->processFields(key);
    socket->sendResponse(b);

    insMode = false;
}

void Keyboard::fKey1()
{
    functionKey(IBM3270_AID_F1);
}

void Keyboard::fKey2()
{
    functionKey(IBM3270_AID_F2);
}

void Keyboard::fKey3()
{
    functionKey(IBM3270_AID_F3);
}

void Keyboard::fKey4()
{
    functionKey(IBM3270_AID_F4);
}

void Keyboard::fKey5()
{
    functionKey(IBM3270_AID_F5);
}

void Keyboard::fKey6()
{
    functionKey(IBM3270_AID_F6);
}

void Keyboard::fKey7()
{
    functionKey(IBM3270_AID_F7);
}

void Keyboard::fKey8()
{
    functionKey(IBM3270_AID_F8);
}

void Keyboard::fKey9()
{
    functionKey(IBM3270_AID_F9);
}

void Keyboard::fKey10()
{
    functionKey(IBM3270_AID_F10);
}

void Keyboard::fKey11()
{
    functionKey(IBM3270_AID_F11);
}

void Keyboard::fKey12()
{
    functionKey(IBM3270_AID_F12);
}

void Keyboard::fKey13()
{
    functionKey(IBM3270_AID_F13);
}

void Keyboard::fKey14()
{
    functionKey(IBM3270_AID_F14);
}

void Keyboard::fKey15()
{
    functionKey(IBM3270_AID_F15);
}

void Keyboard::fKey16()
{
    functionKey(IBM3270_AID_F16);
}

void Keyboard::fKey17()
{
    functionKey(IBM3270_AID_F17);
}

void Keyboard::fKey18()
{
    functionKey(IBM3270_AID_F18);
}

void Keyboard::fKey19()
{
    functionKey(IBM3270_AID_F19);
}

void Keyboard::fKey20()
{
    functionKey(IBM3270_AID_F20);
}

void Keyboard::fKey21()
{
    functionKey(IBM3270_AID_F21);
}

void Keyboard::fKey22()
{
    functionKey(IBM3270_AID_F22);
}

void Keyboard::fKey23()
{
    functionKey(IBM3270_AID_F23);
}

void Keyboard::fKey24()
{
    functionKey(IBM3270_AID_F24);
}

void Keyboard::attn()
{
    Buffer *b = new Buffer();

    b->add(IAC);
    b->add(IP);

    socket->sendResponse(b);

    insMode = false;

    printf("ATTN pressed\n");
}

void Keyboard::programaccessKey(int k)
{
    Buffer *b = new Buffer();

    b->add(k);
    socket->sendResponse(b);

    insMode = false;
}

void Keyboard::paKey1()
{
    programaccessKey(IBM3270_AID_PA1);
}

void Keyboard::paKey2()
{
    programaccessKey(IBM3270_AID_PA2);
}

void Keyboard::paKey3()
{
    programaccessKey(IBM3270_AID_PA3);
}

void Keyboard::newline()
{
    display->newline();
}

void Keyboard::reset()
{
    //TODO: Proper PWAIT/TWAIT handling
    insMode = false;
    lock = false;
    printf("Keyboard unlocked\n");
    fflush(stdout);
    emit setLock(Indicators::Unlocked);
    emit setLock(Indicators::OvertypeMode);
}

void Keyboard::setMapping(int key, QString function)
{

}

void Keyboard::setFactoryMaps()
{

}
