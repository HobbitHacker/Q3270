/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include "ProcessDataStream.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "CodePage.h"
#include "ActiveSettings.h"

#include <QSettings>
#include <QHostInfo>
#include <QGraphicsSimpleTextItem>
#include <QVBoxLayout>
#include <QMenuBar>

class Terminal : public QWidget
{
    Q_OBJECT

    public:

        Terminal(QVBoxLayout *v, ActiveSettings &activeSettings, CodePage &cp, Keyboard &kb, const Colours &cs);
        ~Terminal();

        void connectSession();

        int terminalWidth(bool alternate)       { return(!alternate ? primaryScreen->width() : alternateScreen->width()); }
        int terminalHeight(bool alternate)      { return(!alternate ? primaryScreen->height() : alternateScreen->height()); };
        int gridWidth(bool alternate)           { return(!alternate ? primaryScreen->gridWidth() : alternateScreen->gridWidth()); };
        int gridHeight(bool alternate)          { return(!alternate ? primaryScreen->gridHeight() : alternateScreen->gridHeight()); };

        void setBlink(bool blink);
        void setBlinkSpeed(int speed);

        void setScreenStretch(bool scale);

        DisplayScreen *setAlternateScreen(bool alt);

        void fit();

        inline bool isConnected() { return sessionConnected; };

        QList<QSslCertificate> getCertDetails()    { return socket->getCertDetails(); };

    signals:

        void connectionEstablished();
        void disconnected();
        void windowClosed(Terminal *t);

    public slots:

        void closeConnection(QString message = "");
        void closeEvent(QCloseEvent *closeEvent);
        void setCurrentFont(QFont f);
        void rulerStyle(Q3270::RulerStyle r);
        void rulerChanged(bool on);
        void changeCodePage(QString codepage);
        void setFont(QFont font);

        void resetStatusXSystem();
        void setStatusInsert(const bool insert);
        void setTWait();
        void clearTWait();

        // Set themes by name
        void setColourTheme(const Colours &colours);

        void copyText()                         { current->copyText(); };

        void blinkText();
        void blinkCursor();

    private:

        void connectKeyboard(DisplayScreen &s);
        void disconnectKeyboard(DisplayScreen &s);
        void updateLockState();

        void startTimers();
        void stopTimers();

        Keyboard &kbd;
        CodePage &cp;

        Colours palette;

        ActiveSettings &activeSettings;

        QGraphicsView *view;

        QGraphicsScene *notConnectedScene;
        QGraphicsScene *primary;
        QGraphicsScene *alternate;

        QGraphicsSimpleTextItem *ncReason;

        DisplayScreen *primaryScreen;
        DisplayScreen *alternateScreen;
        DisplayScreen *current;

        ProcessDataStream *datastream;
        SocketConnection *socket;

        bool sessionConnected;

        Qt::AspectRatioMode stretchScreen;

//        QLabel *cursorAddress;
//        QLabel *syslock;
//        QLabel *insMode;

        int blinkSpeed;
        bool blink;

        // Lock conditions
        bool xClock;
        bool xSystem;

        // Whether the timer for a blink operation is short (blink timers when the
        // thing being blinked is hidden are short so the 'off' phase is brief)
        bool shortCursorBlink;
        bool shortCharacterBlink;

        QTimer *blinker;
        QTimer *cursorBlinker;

};

#endif // TERMINAL_H
