#include "keyboard.h"

Keyboard::Keyboard(QObject *parent, DisplayDataStream *d, SocketConnection *c) : QObject(parent)
{    
    display = d;
    socket = c;

    //TODO: Call DisplayDataStream methods directly?
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Up, &Keyboard::cursorUp));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Down, &Keyboard::cursorDown));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Right, &Keyboard::cursorRight));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Left, &Keyboard::cursorLeft));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Enter, &Keyboard::enter));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Tab, &Keyboard::tab));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Home, &Keyboard::home));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_End, &Keyboard::eraseEOF));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Insert, &Keyboard::insert));
    defaultMap.insert(std::pair<int, doSomething>(Qt::Key_Delete, &Keyboard::deleteKey));

    lock = false;
    insMode = false;
}

bool Keyboard::eventFilter( QObject *dist, QEvent *event )
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int key = keyEvent->key();
        std::unordered_map<int, doSomething>::const_iterator got = defaultMap.find(key);
        printf("Keyboard: Count: %d   Key: %d   Native: %d Modifiers: %d   nativeModifiers: %d   nativeVirtualKey: %d    text: %s\n", keyEvent->count(), keyEvent->key(), keyEvent->nativeScanCode(), keyEvent->modifiers(), keyEvent->nativeModifiers(), keyEvent->nativeVirtualKey(), keyEvent->text().toLatin1().data());
        if (got == defaultMap.end())
        {
            display->insertChar(QString(keyEvent->text()), insMode);
        }
        else
        {
            (this->*got->second)();
        }
        return true;
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(dist, event);
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
    lock = true;
    Buffer *b = display->processFields();
    socket->sendResponse(b);
}

void Keyboard::tab()
{
    display->tab();
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
