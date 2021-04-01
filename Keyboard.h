#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QApplication>
#include <QClipboard>
#include <QLineEdit>
#include <QKeyEvent>
#include <QSettings>

//TODO: Change to QMap / QList
#include <unordered_map>
#include <functional>

#include "ProcessDataStream.h"
#include "SocketConnection.h"
#include "TerminalView.h"
#include "3270.h"

//#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

#define Q3270_LEFT_CTRL 65507
#define Q3270_RIGHT_CTRL 65508

class Keyboard : public QObject
{    

    Q_OBJECT

    public:
        Keyboard(ProcessDataStream *d, TerminalView *v);
        void setMap();
        bool processKey();

    signals:
        void setLock(QString xsystem);
        void setInsert(bool ins);
        void saveKeyboardMapping(QString k, QString v);

    public slots:
        void unlockKeyboard();
        void lockKeyboard();
        void setMapping(QString key, QString function);
        void saveKeyboardSettings();

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
        void clear();

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

        void copy();
        void paste();

        void nextKey();
        bool needtoWait(QKeyEvent *q);

        void clearBufferEntry();

        void setFactoryMaps();

        typedef void (Keyboard::*kbFunction)();

        typedef struct
        {
                kbFunction kbFunc;
                QString keySeq;
                QString keyFunc;
        } kbDets;

        QMap<int,kbDets> defaultMap;

        QMap<int, kbDets> altMap;
        QMap<int, kbDets> ctrlMap;
        QMap<int, kbDets> shiftMap;
        QMap<int, kbDets> metaMap;


        typedef struct
        {
            int key;
            int modifiers;
            int nativeKey;
            QChar keyChar;
            QMap<int, kbDets> *map;
            bool isMapped;
            bool mustMap;
            kbFunction mapped;
        } keyStruct;

        keyStruct kbBuffer[1024];

        TerminalView *view;

        int bufferPos;
        int bufferEnd;
        int keyCount;

        QClipboard *clip;       // Clipboard

//        std::unordered_map<QString, doSomething> functionMap;
        QMap<QString, kbFunction> functionMap;
};
#endif // KEYBOARD_H
