#include "Keyboard.h"

Keyboard::Keyboard(ProcessDataStream *d)
{    
    datastream = d;

    lock = false;
    insMode = false;

    bufferPos = 0;
    bufferEnd = 0;
    keyCount  = 0;
    waitRelease = false;

    clearBufferEntry();

    connect(d, &ProcessDataStream::keyboardUnlocked, this, &Keyboard::unlockKeyboard);
}

void Keyboard::setMap()
{
    functionMap.insert(std::pair<QString, doSomething>("Enter",&Keyboard::enter));
    functionMap.insert(std::pair<QString, doSomething>("Reset",&Keyboard::reset));

    functionMap.insert(std::pair<QString, doSomething>("Up",&Keyboard::cursorUp));
    functionMap.insert(std::pair<QString, doSomething>("Down",&Keyboard::cursorDown));
    functionMap.insert(std::pair<QString, doSomething>("Left",&Keyboard::cursorLeft));
    functionMap.insert(std::pair<QString, doSomething>("Right",&Keyboard::cursorRight));

    functionMap.insert(std::pair<QString, doSomething>("Backspace",&Keyboard::cursorLeft));

    functionMap.insert(std::pair<QString, doSomething>("Tab",&Keyboard::tab));
    functionMap.insert(std::pair<QString, doSomething>("Backtab",&Keyboard::backtab));

    functionMap.insert(std::pair<QString, doSomething>("Newline",&Keyboard::newline));
    functionMap.insert(std::pair<QString, doSomething>("Home",&Keyboard::home));

    functionMap.insert(std::pair<QString, doSomething>("EraseEOF",&Keyboard::eraseEOF));

    functionMap.insert(std::pair<QString, doSomething>("Insert",&Keyboard::insert));
    functionMap.insert(std::pair<QString, doSomething>("Delete",&Keyboard::deleteKey));

    functionMap.insert(std::pair<QString, doSomething>("F1",&Keyboard::fKey1));
    functionMap.insert(std::pair<QString, doSomething>("F2",&Keyboard::fKey2));
    functionMap.insert(std::pair<QString, doSomething>("F3",&Keyboard::fKey3));
    functionMap.insert(std::pair<QString, doSomething>("F4",&Keyboard::fKey4));
    functionMap.insert(std::pair<QString, doSomething>("F5",&Keyboard::fKey5));
    functionMap.insert(std::pair<QString, doSomething>("F6",&Keyboard::fKey6));
    functionMap.insert(std::pair<QString, doSomething>("F7",&Keyboard::fKey7));
    functionMap.insert(std::pair<QString, doSomething>("F8",&Keyboard::fKey8));
    functionMap.insert(std::pair<QString, doSomething>("F9",&Keyboard::fKey9));
    functionMap.insert(std::pair<QString, doSomething>("F10",&Keyboard::fKey10));
    functionMap.insert(std::pair<QString, doSomething>("F11",&Keyboard::fKey11));
    functionMap.insert(std::pair<QString, doSomething>("F12",&Keyboard::fKey12));

    functionMap.insert(std::pair<QString, doSomething>("F13",&Keyboard::fKey13));
    functionMap.insert(std::pair<QString, doSomething>("F14",&Keyboard::fKey14));
    functionMap.insert(std::pair<QString, doSomething>("F15",&Keyboard::fKey15));
    functionMap.insert(std::pair<QString, doSomething>("F16",&Keyboard::fKey16));
    functionMap.insert(std::pair<QString, doSomething>("F17",&Keyboard::fKey17));
    functionMap.insert(std::pair<QString, doSomething>("F18",&Keyboard::fKey18));
    functionMap.insert(std::pair<QString, doSomething>("F19",&Keyboard::fKey19));
    functionMap.insert(std::pair<QString, doSomething>("F20",&Keyboard::fKey20));
    functionMap.insert(std::pair<QString, doSomething>("F21",&Keyboard::fKey21));
    functionMap.insert(std::pair<QString, doSomething>("F22",&Keyboard::fKey22));
    functionMap.insert(std::pair<QString, doSomething>("F23",&Keyboard::fKey23));
    functionMap.insert(std::pair<QString, doSomething>("F24",&Keyboard::fKey24));

    functionMap.insert(std::pair<QString, doSomething>("Attn",&Keyboard::attn));

    functionMap.insert(std::pair<QString, doSomething>("PA1",&Keyboard::paKey1));
    functionMap.insert(std::pair<QString, doSomething>("PA2",&Keyboard::paKey2));
    functionMap.insert(std::pair<QString, doSomething>("PA3",&Keyboard::paKey3));

    functionMap.insert(std::pair<QString, doSomething>("Clear",&Keyboard::clear));


    functionMap.insert(std::pair<QString, doSomething>("ToggleRuler",&Keyboard::ruler));

    setFactoryMaps();
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
        printf("%s\n", QKeySequence(keyEvent->key()).toString().toLatin1().data());
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
            keyUsed = processKey();
        } else if (event->type() == QEvent::KeyRelease)
        {
            printf("Processing key - KeyRelease\n");
            fflush(stdout);
            waitRelease = false;
            keyUsed = processKey();
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
            keyUsed = processKey();
        }

    }
    if (keyUsed)
    {
        return true;
    }

    return QObject::eventFilter(dist, event);

}

bool Keyboard::processKey()
{

    printf("Searching for %d in map %ld\n", kbBuffer[bufferEnd].key, kbBuffer[bufferEnd].map);
    fflush(stdout);

    kbMap::const_iterator got;

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
            datastream->insertChar(kbBuffer[bufferPos].keyChar.toUtf8()[0], insMode);
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
    datastream->moveCursor(0, -1);
}

void Keyboard::cursorDown()
{
    datastream->moveCursor(0, 1);
}

void Keyboard::cursorRight()
{
    datastream->moveCursor(1, 0);
}

void Keyboard::cursorLeft()
{
    datastream->moveCursor(-1, 0);
}

void Keyboard::enter()
{
    lockKeyboard();

    datastream->processAID(IBM3270_AID_ENTER, false);

    insMode = false;
}

void Keyboard::tab()
{
    datastream->tab();
}

void Keyboard::backtab()
{
    datastream->backtab();
}

void Keyboard::home()
{
    datastream->home();
}

void Keyboard::eraseEOF()
{
    datastream->eraseField();
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
    datastream->deleteChar();
}

void Keyboard::functionKey(int key)
{
    lockKeyboard();

    datastream->processAID(key, false);

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

    datastream->interruptProcess();

    insMode = false;

    printf("ATTN pressed\n");
}

void Keyboard::programaccessKey(int aidKey)
{
    datastream->processAID(aidKey, true);

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
    datastream->newline();
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

void Keyboard::setMapping(QString key, QString function)
{
    int keyCode;
    kbMap *setMap = &defaultMap;

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
        for (const auto &keyPart: keyList)
        {
            if (!keyPart.compare("Alt", Qt::CaseInsensitive))
            {
                setMap = &altMap;
            }
            else if (!keyPart.compare("Ctrl", Qt::CaseInsensitive))
            {
                setMap = &ctrlMap;
            }
            else if (!keyPart.compare("Shift", Qt::CaseInsensitive))
            {
                setMap =&shiftMap;
            }
            else if (!keyPart.compare("Meta", Qt::CaseInsensitive))
            {
                setMap = &metaMap;
            }
            else
            {
                const QKeySequence keySeq(keyPart);
                if (keySeq.count() == 1)
                {
                    keyCode = keySeq[0];
                }
            }
        }
    }

    std::unordered_map<QString, doSomething>::const_iterator setIterator;

    setIterator = functionMap.find(function);

    if (setIterator == functionMap.end())
    {
        printf("ERROR: Function %s unknown - ignored\n", function.toLatin1().data());
        fflush(stdout);
    }

    setMap->insert(std::pair<int, doSomething>(keyCode, (*setIterator).second));
    emit saveKeyboardMapping("keyboard/" + key, function);
}

void Keyboard::setFactoryMaps()
{
    setMapping("Enter", "Enter");

    setMapping("LCtrl", "Reset");
    setMapping("RCtrl", "Enter");

    setMapping("Insert", "Insert");
    setMapping("Delete", "Delete");

    setMapping("Up", "Up");
    setMapping("Down", "Down");
    setMapping("Left", "Left");
    setMapping("Right", "Right");

    //TODO: make backspace stop at start of field
    setMapping("Backspace","Left");

    setMapping("Tab", "Tab");
    setMapping("Backtab", "Backtab");
    setMapping("Shift+Tab", "Backtab");

    setMapping("Home", "Home");
    setMapping("End", "EraseEOF");
    setMapping("Return", "Newline");

    setMapping("F1", "F1");
    setMapping("F2", "F2");
    setMapping("F3", "F3");
    setMapping("F4", "F4");
    setMapping("F5", "F5");
    setMapping("F6", "F6");
    setMapping("F7", "F7");
    setMapping("F8", "F8");
    setMapping("F9", "F9");
    setMapping("F10", "F10");
    setMapping("F11", "F11");
    setMapping("F12", "F12");

    setMapping("Shift+F1", "F13");
    setMapping("Shift+F2", "F14");
    setMapping("Shift+F3", "F15");
    setMapping("Shift+F4", "F16");
    setMapping("Shift+F5", "F17");
    setMapping("Shift+F6", "F18");
    setMapping("Shift+F7", "F19");
    setMapping("Shift+F8", "F20");
    setMapping("Shift+F9", "F21");
    setMapping("Shift+F10", "F22");
    setMapping("Shift+F11", "F23");
    setMapping("Shift+F12", "F24");

    setMapping("Alt+1", "PA1");
    setMapping("Alt+2", "PA2");
    setMapping("Alt+3", "PA3");

    setMapping("Escape", "Attn");

    setMapping("PageUp", "F7");
    setMapping("PageDown", "F8");

    setMapping("Ctrl+Home", "ToggleRuler");

    setMapping("Pause", "Clear");
}

void Keyboard::ruler()
{
    datastream->toggleRuler();
}

void Keyboard::clear()
{
    datastream->processAID(IBM3270_AID_CLEAR, true);
}
