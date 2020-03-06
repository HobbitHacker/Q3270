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
#define IBM3270_F1    0xF1
#define IBM3270_F2    0xF2
#define IBM3270_F3    0xF3
#define IBM3270_F4    0xF4
#define IBM3270_F5    0xF5
#define IBM3270_F6    0xF6
#define IBM3270_F7    0xF7
#define IBM3270_F8    0xF8
#define IBM3270_F9    0xF9
#define IBM3270_F10   0x7A
#define IBM3270_F11   0x7B
#define IBM3270_F12   0x7C
#define IBM3270_F13   0xC1
#define IBM3270_F14   0xC2
#define IBM3270_F15   0xC3
#define IBM3270_F16   0xC4
#define IBM3270_F17   0xC5
#define IBM3270_F18   0xC6
#define IBM3270_F19   0xC7
#define IBM3270_F20   0xC8
#define IBM3270_F21   0xC9
#define IBM3270_F22   0x4A
#define IBM3270_F23   0x4B
#define IBM3270_F24   0x4C

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

        const unsigned fkeys[24] =
        {
            IBM3270_F1,
            IBM3270_F2,
            IBM3270_F3,
            IBM3270_F4,
            IBM3270_F5,
            IBM3270_F6,
            IBM3270_F7,
            IBM3270_F8,
            IBM3270_F9,
            IBM3270_F10,
            IBM3270_F11,
            IBM3270_F12,
            IBM3270_F13,
            IBM3270_F14,
            IBM3270_F15,
            IBM3270_F16,
            IBM3270_F17,
            IBM3270_F18,
            IBM3270_F19,
            IBM3270_F20,
            IBM3270_F21,
            IBM3270_F22,
            IBM3270_F23,
            IBM3270_F24
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
        void functionkey();

        std::unordered_map<int, doSomething> defaultMap;
};

#endif // KEYBOARD_H
