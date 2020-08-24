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

    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F1, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F2, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F3, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F4, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F5, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F6, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F7, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F8, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F9, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F10, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F11, &Keyboard::functionkey));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_F12, &Keyboard::functionkey));

    lock = false;
    insMode = false;

    bufferPos = 0;
    bufferEnd = 0;
    keyCount  = 0;

    connect(d, &DisplayDataStream::keyboardUnlocked, this, &Keyboard::unlockKeyboard);
    printf("Keyboard unlocked\n");
    fflush(stdout);
}

void Keyboard::lockKeyboard()
{
    lock = true;
    printf("Keyboard locked\n");
    fflush(stdout);
}

void Keyboard::unlockKeyboard()
{
    lock = false;
    printf("Keyboard unlocked\n");
    fflush(stdout);
    nextKey();
}

bool Keyboard::eventFilter( QObject *dist, QEvent *event )
{
    if (event->type() != QEvent::KeyPress) {
        // standard event processing
        return QObject::eventFilter(dist, event);
    }

    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    key = keyEvent->key();
    modifier = keyEvent->modifiers();
    std::unordered_map<int, doSomething>::const_iterator got = defaultMap.find(key);
    printf("Keyboard: Count: %d   Key: %d   Native: %d Modifiers: %d   nativeModifiers: %d   nativeVirtualKey: %d    text: %s\n", keyEvent->count(), keyEvent->key(), keyEvent->nativeScanCode(), keyEvent->modifiers(), keyEvent->nativeModifiers(), keyEvent->nativeVirtualKey(), keyEvent->text().toLatin1().data());

    if (keyEvent->text() != "" | got != defaultMap.end())
    {
        kbBuffer[bufferEnd].key = key;
        kbBuffer[bufferEnd].modifiers = modifier;
        kbBuffer[bufferEnd].keyChar = keyEvent->text();

        if (got == defaultMap.end())
        {
            kbBuffer[bufferEnd].isMapped = false;
        }
        else
        {
            kbBuffer[bufferEnd].isMapped = true;
            kbBuffer[bufferEnd].mapped = got->second;
        }

        keyCount++;

        if (!lock) {
            nextKey();
        } else {
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
    printf("Insert mode now %d\n", insMode);
    fflush(stdout);
}

void Keyboard::deleteKey()
{
    display->deleteChar();
}

void Keyboard::functionkey()
{
    int fkeyAdjusted = key - Qt::Key_F1;

    if (modifier == Qt::ShiftModifier)
    {
        fkeyAdjusted = fkeyAdjusted + 12;
    }
    lockKeyboard();
    Buffer *b = display->processFields(fkeys[fkeyAdjusted]);
    socket->sendResponse(b);

    insMode = false;
}

void Keyboard::newline()
{
    display->newline();
}
