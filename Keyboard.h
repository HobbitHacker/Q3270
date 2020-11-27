#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QApplication>
#include <QLineEdit>
#include <QKeyEvent>
#include <unordered_map>
#include <functional>

#include "ProcessDataStream.h"
#include "SocketConnection.h"
#include "Buffer.h"
#include "3270.h"

//#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define Q3270_LEFT_CTRL 65507
#define Q3270_RIGHT_CTRL 65508

class Keyboard : public QObject
{    

    Q_OBJECT

    typedef void (Keyboard::*doSomething)();

    public:
        Keyboard(ProcessDataStream *d = 0);
        void setMap();
        bool processKey();

    signals:
        void setLock(Indicators i);
        void saveKeyboardMapping(QString k, QString v);

    public slots:
        void unlockKeyboard();
        void lockKeyboard();
        void setMapping(QString key, QString function);

    protected:
        bool eventFilter( QObject *dist, QEvent *event );

    private:

        ProcessDataStream *datastream;

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

        void ruler();

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

        typedef std::unordered_map<int, doSomething> kbMap;

        kbMap defaultMap;
        kbMap altMap;
        kbMap ctrlMap;
        kbMap shiftMap;
        kbMap metaMap;

        typedef struct
        {
            int key;
            int modifiers;
            int nativeKey;
            QString keyChar;
            kbMap *map;
            bool isMapped;
            bool mustMap;
            doSomething mapped;
        } keyStruct;

        keyStruct kbBuffer[1024];

        int bufferPos;
        int bufferEnd;
        int keyCount;

        std::unordered_map<QString, doSomething> functionMap;
};
#endif // KEYBOARD_H