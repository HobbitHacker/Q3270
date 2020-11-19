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

#define Q3270_LEFT_CTRL 65507
#define Q3270_RIGHT_CTRL 65508

class Keyboard : public QObject
{    

    Q_OBJECT

    typedef void (Keyboard::*doSomething)();

    public:
        Keyboard(DisplayDataStream *d = 0, SocketConnection *c = 0);
        bool processKey(QKeyEvent *qk);

    signals:
        void setLock(Indicators i);

    public slots:
        void unlockKeyboard();
        void lockKeyboard();
        void setMapping(int key, QString function);

    protected:
        bool eventFilter( QObject *dist, QEvent *event );

    private:

        DisplayDataStream *display;
        SocketConnection *socket;

        bool lock;
        bool insMode;
        bool waitRelease;

        void cursorUp();
        void cursorDown();
        void cursorRight();
        void cursorLeft();
        void enter();
        void tab();
        void backtab();
        void home();
        void insert();
        void reset();
        void deleteKey();
        void backspace();
        void eraseEOF();
        void newline();
        void functionKey(int k);
        void programaccessKey(int k);
        void attn();

        void fKey1();
        void fKey2();
        void fKey3();
        void fKey4();
        void fKey5();
        void fKey6();
        void fKey7();
        void fKey8();
        void fKey9();
        void fKey10();
        void fKey11();
        void fKey12();
        void fKey13();
        void fKey14();
        void fKey15();
        void fKey16();
        void fKey17();
        void fKey18();
        void fKey19();
        void fKey20();
        void fKey21();
        void fKey22();
        void fKey23();
        void fKey24();

        void paKey1();
        void paKey2();
        void paKey3();

        void nextKey();
        bool needtoWait(QKeyEvent *q);

        void clearBufferEntry();

        void setFactoryMaps();

        std::unordered_map<int, doSomething> defaultMap;
        std::unordered_map<int, doSomething> altMap;
        std::unordered_map<int, doSomething> ctrlMap;
        std::unordered_map<int, doSomething> shiftMap;
        std::unordered_map<int, doSomething> metaMap;


        typedef struct
        {
            int key;
            int modifiers;
            int nativeKey;
            QString keyChar;
            std::unordered_map<int, doSomething> *map;
            bool isMapped;
            bool mustMap;
            doSomething mapped;
        } keyStruct;



        keyStruct kbBuffer[1024];

        int bufferPos;
        int bufferEnd;
        int keyCount;
};

#endif // KEYBOARD_H
