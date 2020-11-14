#include "keyboard.h"

Keyboard::Keyboard(DisplayDataStream *d, SocketConnection *c)
{    
    display = d;
    socket = c;

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

    altMap.insert(std::pair<int, doSomething>(Qt::Key_1, &Keyboard::paKey1));
    altMap.insert(std::pair<int, doSomething>(Qt::Key_2, &Keyboard::paKey2));
    altMap.insert(std::pair<int, doSomething>(Qt::Key_3, &Keyboard::paKey3));

    lock = false;
    insMode = false;

    bufferPos = 0;
    bufferEnd = 0;
    keyCount  = 0;
    waitRelease = false;

    connect(d, &DisplayDataStream::keyboardUnlocked, this, &Keyboard::unlockKeyboard);
    printf("Keyboard unlocked\n");
    fflush(stdout);
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
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        printf("Keyboard: Type: %s   Count: %d   Key: %d   Native: %d Modifiers: %d   nativeModifiers: %d   nativeVirtualKey: %d    text: %s    Mapped:%d\n", event->type() == QEvent::KeyPress ? "Press" : "Release", keyEvent->count(), keyEvent->key(), keyEvent->nativeScanCode(), keyEvent->modifiers(), keyEvent->nativeModifiers(), keyEvent->nativeVirtualKey(), keyEvent->text().toLatin1().data(), kbBuffer[bufferEnd].isMapped);
        fflush(stdout);

        // If we need to wait for a key to be released (from a previous key press)
        // check to see if the event was a key press. If so, and it generated a character,
        // don't wait. This is to stop shifted letters waiting for their release, but allowing
        // an F-key to wait for its press.

        bool keyUsed = false;

        if (waitRelease)
        {
            if  (event->type() == QEvent::KeyPress && keyEvent->text() != "")
            {
                waitRelease = false;
                keyUsed = processKey(keyEvent);
            }
            if (event->type() == QEvent::KeyRelease)
            {
                waitRelease = false;
                keyUsed = processKey(keyEvent);
            }
        }
        else if (!waitRelease && event->type() == QEvent::KeyPress)
        {
            if (keyEvent->text() == "" && needtoWait(keyEvent))
            {
                waitRelease = true;
                keyUsed = true;
            }
            else
            {
                keyUsed = processKey(keyEvent);
            }

        }
        if (keyUsed)
        {
            return true;
        }
    }

    return QObject::eventFilter(dist, event);
}

bool Keyboard::processKey(QKeyEvent *keyEvent)
{
    if (keyEvent->type() == QEvent::KeyPress && keyEvent->text() != "")
    {
        needtoWait(keyEvent);
    }

    std::unordered_map<int, doSomething>::const_iterator got;

    kbBuffer[bufferEnd].isMapped = false;

    got = kbBuffer[bufferEnd].map->find(kbBuffer[bufferEnd].key);

    kbBuffer[bufferEnd].isMapped = (got != kbBuffer[bufferEnd].map->end());

    printf("Mapped: %d\n", kbBuffer[bufferEnd].isMapped);

    if (!kbBuffer[bufferEnd].isMapped && keyEvent->type() == QEvent::KeyRelease)
    {
        got = kbBuffer[bufferEnd].map->find(kbBuffer[bufferEnd].nativeKey);
        kbBuffer[bufferEnd].isMapped = (got != kbBuffer[bufferEnd].map->end());
        printf(" - Modifier Mapped: %d\n", kbBuffer[bufferEnd].isMapped);
    }
    fflush(stdout);

    /*
    switch(kbBuffer[bufferEnd].modifiers)
    {
        case Qt::AltModifier:
            got = altMap.find(kbBuffer[bufferEnd].key);
            kbBuffer[bufferEnd].isMapped = (got != altMap.end());
            if (!kbBuffer[bufferEnd].isMapped && keyEvent->type() == QEvent::KeyRelease)
            {
                got = altMap.find(kbBuffer[bufferEnd].nativeKey);
                kbBuffer[bufferEnd].isMapped = (got != altMap.end());
            }
            break;
        case Qt::ShiftModifier:
            got = shiftMap.find(kbBuffer[bufferEnd].key);
            kbBuffer[bufferEnd].isMapped = (got != shiftMap.end());
            if (!kbBuffer[bufferEnd].isMapped && keyEvent->type() == QEvent::KeyRelease)
            {
                got = shiftMap.find(kbBuffer[bufferEnd].nativeKey);
                kbBuffer[bufferEnd].isMapped = (got != shiftMap.end());
            }
            break;
        case Qt::ControlModifier:
            got = ctrlMap.find(kbBuffer[bufferEnd].key);
            kbBuffer[bufferEnd].isMapped = (got != ctrlMap.end());
            if (!kbBuffer[bufferEnd].isMapped && keyEvent->type() == QEvent::KeyRelease)
            {
                got = ctrlMap.find(kbBuffer[bufferEnd].nativeKey);
                kbBuffer[bufferEnd].isMapped = (got != ctrlMap.end());
            }
            break;
        case Qt::NoModifier:
        case Qt::KeypadModifier:
            got = defaultMap.find(kbBuffer[bufferEnd].key);
            kbBuffer[bufferEnd].isMapped = (got != defaultMap.end());
            if (!kbBuffer[bufferEnd].isMapped && keyEvent->type() == QEvent::KeyRelease)
            {
                got = defaultMap.find(kbBuffer[bufferEnd].nativeKey);
                kbBuffer[bufferEnd].isMapped = (got != defaultMap.end());
            }
            break;
    }
*/
    if (keyEvent->text() != "" || kbBuffer[bufferEnd].isMapped) {

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
    }
    return true;
}


bool Keyboard::needtoWait(QKeyEvent *event)
{
    kbBuffer[bufferEnd].key = event->key();
    kbBuffer[bufferEnd].keyChar = event->text();
    kbBuffer[bufferEnd].nativeKey = event->nativeVirtualKey();
    kbBuffer[bufferEnd].modifiers = event->modifiers();
    kbBuffer[bufferEnd].mustMap = false;

    if (ctrlMap.size() > 0 && event->modifiers() == Qt::ControlModifier) {
        kbBuffer[bufferEnd].map = &ctrlMap;
        kbBuffer[bufferEnd].mustMap = true;
        return true;
    }
    if (altMap.size() > 0 && event->modifiers() == Qt::AltModifier) {
        kbBuffer[bufferEnd].map = &altMap;
        kbBuffer[bufferEnd].mustMap = true;
        return true;
    }
    if (shiftMap.size() > 0 && event->modifiers() == Qt::ShiftModifier) {
        kbBuffer[bufferEnd].map = &shiftMap;
        return true;
    }
    kbBuffer[bufferEnd].map = &defaultMap;
    return false;
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
