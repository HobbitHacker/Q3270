#include "Keyboard.h"

Keyboard::Keyboard(KeyboardTheme *keyboardTheme) : keyboardTheme(keyboardTheme)
{    
    lock = false;
    insMode = false;

    bufferPos = 0;
    bufferEnd = 0;
    keyCount  = 0;
    waitRelease = false;

    clearBufferEntry();
    setMap();
}

void Keyboard::setDataStream(ProcessDataStream *d)
{
    datastream = d;
    connect(datastream, &ProcessDataStream::keyboardUnlocked, this, &Keyboard::unlockKeyboard);
}

void Keyboard::setMap()
{
    functionMap.insert("Enter",&Keyboard::enter);
    functionMap.insert("Reset",&Keyboard::reset);

    functionMap.insert("Up",&Keyboard::cursorUp);
    functionMap.insert("Down",&Keyboard::cursorDown);
    functionMap.insert("Left",&Keyboard::cursorLeft);
    functionMap.insert("Right",&Keyboard::cursorRight);

    functionMap.insert("Backspace",&Keyboard::backspace);

    functionMap.insert("Tab",&Keyboard::tab);
    functionMap.insert("Backtab",&Keyboard::backtab);

    functionMap.insert("NewLine",&Keyboard::newline);
    functionMap.insert("Home",&Keyboard::home);
    functionMap.insert("EndLine", &Keyboard::endline);

    functionMap.insert("EraseEOF",&Keyboard::eraseEOF);

    functionMap.insert("Insert",&Keyboard::insert);
    functionMap.insert("Delete",&Keyboard::deleteKey);

    functionMap.insert("F1",&Keyboard::fKey1);
    functionMap.insert("F2",&Keyboard::fKey2);
    functionMap.insert("F3",&Keyboard::fKey3);
    functionMap.insert("F4",&Keyboard::fKey4);
    functionMap.insert("F5",&Keyboard::fKey5);
    functionMap.insert("F6",&Keyboard::fKey6);
    functionMap.insert("F7",&Keyboard::fKey7);
    functionMap.insert("F8",&Keyboard::fKey8);
    functionMap.insert("F9",&Keyboard::fKey9);
    functionMap.insert("F10",&Keyboard::fKey10);
    functionMap.insert("F11",&Keyboard::fKey11);
    functionMap.insert("F12",&Keyboard::fKey12);

    functionMap.insert("F13",&Keyboard::fKey13);
    functionMap.insert("F14",&Keyboard::fKey14);
    functionMap.insert("F15",&Keyboard::fKey15);
    functionMap.insert("F16",&Keyboard::fKey16);
    functionMap.insert("F17",&Keyboard::fKey17);
    functionMap.insert("F18",&Keyboard::fKey18);
    functionMap.insert("F19",&Keyboard::fKey19);
    functionMap.insert("F20",&Keyboard::fKey20);
    functionMap.insert("F21",&Keyboard::fKey21);
    functionMap.insert("F22",&Keyboard::fKey22);
    functionMap.insert("F23",&Keyboard::fKey23);
    functionMap.insert("F24",&Keyboard::fKey24);

    functionMap.insert("Attn",&Keyboard::attn);

    functionMap.insert("PA1",&Keyboard::paKey1);
    functionMap.insert("PA2",&Keyboard::paKey2);
    functionMap.insert("PA3",&Keyboard::paKey3);

    functionMap.insert("Clear",&Keyboard::clear);

    functionMap.insert("ToggleRuler",&Keyboard::ruler);

    functionMap.insert("Copy",&Keyboard::copy);
    functionMap.insert("Paste",&Keyboard::paste);
    functionMap.insert("Info", &Keyboard::info);

    functionMap.insert("Blah", &Keyboard::unlockKeyboard);

    setFactoryMaps();
}

void Keyboard::clearBufferEntry()
{
    kbBuffer[bufferEnd].modifiers = Qt::NoModifier;
    kbBuffer[bufferEnd].keyChar = QChar(00);
    kbBuffer[bufferEnd].mustMap = false;
    kbBuffer[bufferEnd].isMapped = false;
    kbBuffer[bufferEnd].key = 0;
}

void Keyboard::lockKeyboard()
{
    lock = true;
    printf("Keyboard        : Keyboard locked\n");
    fflush(stdout);
    emit setLock("X System");
}

void Keyboard::unlockKeyboard()
{
    lock = false;
    printf("Keyboard        : Keyboard unlocked\n");
    fflush(stdout);
    emit setLock("");
    nextKey();
}

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
    // don't wait. This is to stop shifted letters waiting for their release, but allowing
    // an F-key to wait for its press.

    bool keyUsed = false;
    if (keyEvent->type() == QEvent::KeyPress)
    {
        waitRelease = needtoWait(keyEvent);
        if (!waitRelease)
        {
            printf("Keyboard        : Processing Key - No need to wait\n");
            fflush(stdout);
            keyUsed = processKey();
        }

    }
    else if (waitRelease)
    {
        printf("Keyboard        : Processing key - KeyRelease\n");
        fflush(stdout);
        waitRelease = false;
        keyUsed = processKey();
    }
    else
    {
        printf("Keyboard        : Ignoring KeyRelease (already processed press)\n");
        fflush(stdout);
    }

    if (keyUsed)
    {
        return true;
    }

    return QObject::eventFilter(dist, event);

}

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
        printf("Keyboard        : Modifier key is mapped: %d\n", kbBuffer[bufferEnd].isMapped);
    }
    fflush(stdout);

    keyCount++;
    printf("Keyboard        : Keycount incremented. Key: %d isMapped: %d, mustMap: %d\n", kbBuffer[bufferEnd].key, kbBuffer[bufferEnd].isMapped, kbBuffer[bufferEnd].mustMap);
    fflush(stdout);

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
        else if (!lock)
        {
           if (kbBuffer[bufferEnd].mustMap && kbBuffer[bufferEnd].isMapped)
           {
               printf("Keyboard        : Processing nextKey (mapped and mustMap set)\n");
               fflush(stdout);
               nextKey();
           }
           else if (!kbBuffer[bufferEnd].mustMap)
           {
               printf("Keyboard        : Processing nextKey (not mapped, and mustMap not set)\n");
               fflush(stdout);
               nextKey();
           }
           else
           {
               printf("Keyboard        : Ignoring key\n");
               fflush(stdout);
               keyCount--;
               clearBufferEntry();
               return false;
           }
        }
        else
        {
            printf("Keyboard        : Locked, cannot process, buffering\n");
            fflush(stdout);

            bufferEnd++;

            if (bufferEnd > 1023) {
                bufferEnd = 0;
            }

            clearBufferEntry();
        }
    } else
    {
        printf("Keyboard        : Ignoring key\n");
        fflush(stdout);
        keyCount--;
        clearBufferEntry();
        return false;
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
//            printf("Keyboard        : Storing modifiers %8.8x\n", event->modifiers());
            kbBuffer[bufferEnd].modifiers = event->modifiers();
            kbBuffer[bufferEnd].nativeKey = event->nativeVirtualKey();
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
    fflush(stdout);

    switch(kbBuffer[bufferEnd].modifiers)
    {
        case Qt::ControlModifier:
            printf("Keyboard        : Switching to CTRL map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].map = &ctrlMap;
            kbBuffer[bufferEnd].mustMap = true;
            break;

        case Qt::AltModifier:
            printf("Keyboard        : Switching to ALT map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].map = &altMap;
            kbBuffer[bufferEnd].mustMap = true;
            break;

        case Qt::ShiftModifier:
            printf("Keyboard        : Switching to SHIFT map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].map = &shiftMap;
            kbBuffer[bufferEnd].mustMap = false;
            break;

        default:
            printf("Keyboard        : Switching to default map\n");
            fflush(stdout);
            kbBuffer[bufferEnd].mustMap = false;
            kbBuffer[bufferEnd].map = &defaultMap;
            break;
    }

    return wait;
}

void Keyboard::nextKey()
{
    if (keyCount > 0)
    {
        printf("Keyboard        : Keycount now %d (buffer pos %d, buffer end %d)\n", keyCount, bufferPos, bufferEnd);
        fflush(stdout);
        keyCount--;
        if (kbBuffer[bufferPos].isMapped)
        {
            (this->*kbBuffer[bufferPos].mapped)();
        }
        else
        {
            // TODO: Might break
            datastream->insertChar(kbBuffer[bufferPos].keyChar.toLatin1(), insMode);
        }
        clearBufferEntry();
        if (bufferEnd != bufferPos)
        {
            bufferPos++;
            if (bufferPos > 1023)
            {
                bufferPos = 0;
            }
            if (!lock && keyCount > 0)
            {
                printf("Keyboard        : processing next key (more stored)\n");
                fflush(stdout);
                nextKey();
            }
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

void Keyboard::backspace()
{
    datastream->backspace();
}

void Keyboard::enter()
{
    lockKeyboard();

    datastream->processAID(IBM3270_AID_ENTER, false);

    insMode = false;
    emit setInsert(false);
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
        emit setInsert(true);
    } else {
        emit setInsert(false);
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
    emit setInsert(false);
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
    emit setInsert(false);

    printf("Keyboard        : ATTN pressed\n");
}

void Keyboard::programaccessKey(int aidKey)
{
    datastream->processAID(aidKey, true);

    insMode = false;
    emit setInsert(false);
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
    printf("Keyboard        : Keyboard unlocked\n");
    fflush(stdout);
    emit setLock("");
    emit setInsert(false);
}

void Keyboard::endline()
{
    datastream->endline();
}

void Keyboard::clear()
{
    datastream->processAID(IBM3270_AID_CLEAR, true);
}

void Keyboard::copy()
{
    emit copyText();
}

void Keyboard::ruler()
{
    datastream->toggleRuler();
}

void Keyboard::paste()
{
    clip = QApplication::clipboard();

    QString clipText = clip->text();
    int clipWidth = 0;

    for(int i = 0; i < clipText.length(); i++)
    {
        if (clipText.at(i) == '\n')
        {
            datastream->moveCursor(0 - clipWidth, 1);
            clipWidth = 0;
        }
        else
        {
            datastream->insertChar(clipText.at(i).toLatin1(), insMode);
            clipWidth++;
        }
    }
}

void Keyboard::info()
{
    datastream->showInfo();
}

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
        printf("Keyboard        : ERROR: Function %s unknown - ignored\n", function.toLatin1().data());
        fflush(stdout);
    }

    setMap->insert(keyCode, { functionMap.value(function), key, function });

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

    setMapping("Backspace","Backspace");

    setMapping("Tab", "Tab");
    setMapping("Backtab", "Backtab");
    setMapping("Shift+Tab", "Backtab");
    setMapping("Shift+Backtab", "Backtab");

    setMapping("Home", "Home");
    setMapping("End", "EraseEOF");
    setMapping("Return", "NewLine");
    setMapping("Ctrl+End", "EndLine");

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

    setMapping("PgUp", "F7");
    setMapping("PgDown", "F8");

    setMapping("Ctrl+Home", "ToggleRuler");

    setMapping("Pause", "Clear");

    setMapping("Ctrl+C", "Copy");
    setMapping("Ctrl+V", "Paste");
    setMapping("Ctrl+I", "Info");
}

void Keyboard::setTheme(QString theme)
{
    // Switch to a new keyboard map based on the theme

    // Clear the existing maps
    defaultMap.clear();
    ctrlMap.clear();
    altMap.clear();
    shiftMap.clear();
    metaMap.clear();

    // Keyboard themes are defined as { Q3270 function, { key, key, key } }

    KeyboardTheme::KeyboardMap kbm = keyboardTheme->getTheme(theme);

    KeyboardTheme::KeyboardMap::ConstIterator i = kbm.constBegin();

    // Iterate over the keyboard theme, and apply
    while(i != kbm.constEnd())
    {
        // Each Q3270 function in the map may have multiple keys defined for it
        for (int s = 0; s < i.value().size(); s++)
        {
            // Set mapping for this key to this function
            setMapping(i.value()[s], i.key());
        }
        i++;
    }
}

void Keyboard::setConnected(bool state)
{
    connectedState = state;
}
