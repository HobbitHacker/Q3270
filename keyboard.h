#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QApplication>
#include <QLineEdit>
#include <QKeyEvent>
#include <unordered_map>
#include <functional>

#include "DisplayDataStream.h"
#include "SocketConnection.h"
#include "buffer.h"

#define IBM3270_ENTER 0x7D

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

class Keyboard : public QObject
{
    typedef void (Keyboard::*doSomething)();

    public:
        Keyboard(QObject *parent = 0, DisplayDataStream *d = 0, SocketConnection *c = 0);

    protected:
        bool eventFilter( QObject *dist, QEvent *event );

    signals:


    private:

        DisplayDataStream *display;
        SocketConnection *socket;

        bool lock;
        bool insMode;

        void cursorUp();
        void cursorDown();
        void cursorRight();
        void cursorLeft();
        void enter();
        void tab();
        void home();
        void insert();
        void reset();
        void deleteKey();
        void backspace();
        void eraseEOF();

        std::unordered_map<int, doSomething> defaultMap;
};

#endif // KEYBOARD_H
