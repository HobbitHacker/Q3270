/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <QApplication>
#include <QClipboard>
#include <QLineEdit>
#include <QKeyEvent>
#include <QSettings>
#include <QVector>
#include <QMap>

#include "Q3270.h"
#include "Models/KeyboardMap.h"

class Keyboard : public QObject
{    

    Q_OBJECT

    public:

        Keyboard();

        bool processKey();

        static QStringList allFunctionNames();

        // Lookup helper
        void invoke(const QString &functionName);
        void setLocked(const bool locked);


    signals:
        void setEnterInhibit();
        void setInsert(bool ins);

        void key_Copy();
        void key_moveCursor(int x, int y, bool absolute);
        void key_Backspace();
        void key_AID(int aid, bool short_read);
        void key_Attn();
        void key_Tab(int offset);
        void key_Backtab();
        void key_Home();
        void key_EraseEOF();
        void key_Delete();
        void key_Newline();
        void key_End();
        void key_toggleRuler();
        void key_Character(unsigned char keycode, bool insMode);
        void key_Reset();

        void key_showInfo();
        void key_showFields();
        void key_dumpScreen();

    public slots:
//        void unlockKeyboard();
        void setConnected(bool state);

        void setMap(const KeyboardMap &kmap);

    protected:
        bool eventFilter( QObject *dist, QEvent *event );

    private:
        using Handler = void (Keyboard::*)();

        struct FunctionBinding {
            const char *name;
            Handler method;
        };

        static const FunctionBinding bindings[];
        static QMap<QString, Handler> makeFunctionMap();
        const QMap<QString, Handler> functionMap;

        struct kbDets
        {
                Handler kbFunc;
                QString keySeq;
                QString keyFunc;
        };

        struct keyStruct
        {
            int key;
            int modifiers;
            int nativeKey;
            QChar keyChar;
            QMap<int, kbDets> *map;
            bool isMapped;
            bool mustMap;
            Handler mapped;
        };

        QClipboard *clip;       // Clipboard

        QMap<int, kbDets> defaultMap;
        QMap<int, kbDets> altMap;
        QMap<int, kbDets> ctrlMap;
        QMap<int, kbDets> shiftMap;
        QMap<int, kbDets> metaMap;

        keyStruct kbBuffer[1024];

        int bufferPos;
        int bufferEnd;
        int keyCount;

        bool systemLock;

        bool insMode;
        bool waitRelease;
        bool connectedState;        // Whether the session is connected or not

        void lockKeyboard();

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
        void endline();

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
        void info();
        void fields();
        void dumpscreen();

        void nextKey();
        bool needtoWait(QKeyEvent *q);

        void clearBufferEntry();

        void setMapping(QString key, QString function);
};
#endif // KEYBOARD_H
