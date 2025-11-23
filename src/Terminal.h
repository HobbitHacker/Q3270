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

#include <QSettings>
#include <QHostInfo>
#include <QGraphicsSimpleTextItem>
#include <QMenuBar>

#include "ProcessDataStream.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "CodePage.h"
#include "ActiveSettings.h"
#include "Display/StatusBar.h"

class Terminal : public QWidget
{
    Q_OBJECT

    public:

        Terminal(QGraphicsView *screen, ActiveSettings &activeSettings, CodePage &cp, Keyboard &kb, const Colours &cs);
        ~Terminal();

        void connectSession();

        // int terminalWidth(bool alternate)       { return(!alternate ? primaryScreen->width() : alternateScreen->width()); }
        // int terminalHeight(bool alternate)      { return(!alternate ? primaryScreen->height() : alternateScreen->height()); };
        // int gridWidth(bool alternate)           { return(!alternate ? primaryScreen->gridWidth() : alternateScreen->gridWidth()); };
        // int gridHeight(bool alternate)          { return(!alternate ? primaryScreen->gridHeight() : alternateScreen->gridHeight()); };
        int terminalWidth(bool alternate)          { return(!alternate ? 80 : activeSettings.getTerminalX()); };
        int terminalHeight(bool alternate)         { return(!alternate ? 24 : activeSettings.getTerminalY()); };
        int gridWidth(bool alternate)              { return(!alternate ? 80 *  12 : activeSettings.getTerminalX() * 12); };
        int gridHeight(bool alternate)             { return(!alternate ? 24 *  22 : activeSettings.getTerminalY() * 22); };

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
        void closeEvent(QCloseEvent *closeEvent) override;
        void setCurrentFont(QFont f);
        void rulerStyle(Q3270::RulerStyle r);
        void rulerChanged(bool on);
        void changeCodePage(QString codepage);
        void setFont(QFont font);

        void toggleRuler();

        void resetStatusXSystem();
        void setStatusInsert(const bool insert);
        void setTWait();
        void clearTWait();

        // Set themes by name
        void setColourTheme(const Colours &colours);

        void copyText()                         { current->copyText(); };

        void blinkText();
        void blinkCursor();

        bool eventFilter(QObject* obj, QEvent* event);
        
    private:

        void connectKeyboard();
        void disconnectKeyboard();
        void updateLockState();

        void startTimers();
        void stopTimers();

        Keyboard &kbd;
        CodePage &cp;

        Colours palette;

        ActiveSettings &activeSettings;

        QGraphicsView *screen;

        QGraphicsRectItem *notConnected;
        QGraphicsSimpleTextItem *ncReason;

        DisplayScreen *current;

        StatusBar *statusBar;

        ProcessDataStream *datastream;
        SocketConnection *socket;

        bool sessionConnected;

        Qt::AspectRatioMode stretchScreen;

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
