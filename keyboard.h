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
#include "3270.h"

//#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

class Keyboard : public QObject
{
    typedef void (Keyboard::*doSomething)();

    public:
        Keyboard(DisplayDataStream *d = 0, SocketConnection *c = 0);

    protected:
        bool eventFilter( QObject *dist, QEvent *event );

    signals:


    private:

        const unsigned fkeys[24] =
        {
            IBM3270_AID_F1,
            IBM3270_AID_F2,
            IBM3270_AID_F3,
            IBM3270_AID_F4,
            IBM3270_AID_F5,
            IBM3270_AID_F6,
            IBM3270_AID_F7,
            IBM3270_AID_F8,
            IBM3270_AID_F9,
            IBM3270_AID_F10,
            IBM3270_AID_F11,
            IBM3270_AID_F12,
            IBM3270_AID_F13,
            IBM3270_AID_F14,
            IBM3270_AID_F15,
            IBM3270_AID_F16,
            IBM3270_AID_F17,
            IBM3270_AID_F18,
            IBM3270_AID_F19,
            IBM3270_AID_F20,
            IBM3270_AID_F21,
            IBM3270_AID_F22,
            IBM3270_AID_F23,
            IBM3270_AID_F24
        };

        DisplayDataStream *display;
        SocketConnection *socket;

        bool lock;
        bool insMode;
        int key;
        Qt::KeyboardModifiers modifier;

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
        void newline();
        void functionkey();

        std::unordered_map<int, doSomething> defaultMap;
};

#endif // KEYBOARD_H
