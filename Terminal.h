/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TERMINAL_H
#define TERMINAL_H

#include "ProcessDataStream.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "ColourTheme.h"
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

    Terminal(QVBoxLayout *v, ActiveSettings &activeSettings, CodePage &cp, Keyboard &kb, ColourTheme &cs, QString sessionName);
        ~Terminal();

        void openConnection(QString address);
        void openConnection(QSettings& s);

        int terminalWidth(bool alternate)       { return(!alternate ? primaryScreen->width() : alternateScreen->width()); }
        int terminalHeight(bool alternate)      { return(!alternate ? primaryScreen->height() : alternateScreen->height()); };
        int gridWidth(bool alternate)           { return(!alternate ? primaryScreen->gridWidth() : alternateScreen->gridWidth()); };
        int gridHeight(bool alternate)          { return(!alternate ? primaryScreen->gridHeight() : alternateScreen->gridHeight()); };

        void setBlink(bool blink);
        void setBlinkSpeed(int speed);

        void setScreenStretch(bool scale);

        void setKeyboardTheme(QString themeName);
        
        // Return current session name
        inline QString getSessionName()    { return sessionName; };
        inline void    setSessionName(QString sessionName) { this->sessionName = sessionName; };

        DisplayScreen *setAlternateScreen(bool alt);

        void fit();

    signals:

        void connectionEstablished();
        void disconnected();
        void windowClosed(Terminal *t);

    public slots:

        void closeConnection();
        void closeEvent(QCloseEvent *closeEvent);
        void setCurrentFont(QFont f);
        void rulerStyle(int r);
        void rulerChanged(bool on);
        void changeCodePage();
        void setFont(QFont font);

        // Set themes by name
        void setColourTheme(QString themeName);

        void copyText()                         { current->copyText(); };

        void blinkText();
        void blinkCursor();

    private:

        void connectSession(QString host, int port, QString luName);
        void connectKeyboard(DisplayScreen &s);
        void disconnectKeyboard(DisplayScreen &s);

        void startTimers();
        void stopTimers();

        Keyboard &kbd;
        ColourTheme &colourtheme;
        CodePage &cp;

        ColourTheme::Colours palette;

        ActiveSettings &activeSettings;

        QGraphicsView *view;

        QGraphicsScene *notConnectedScene;
        QGraphicsScene *primary;
        QGraphicsScene *alternate;

        DisplayScreen *primaryScreen;
        DisplayScreen *alternateScreen;
        DisplayScreen *current;

        ProcessDataStream *datastream;
        SocketConnection *socket;

        bool sessionConnected;

        Qt::AspectRatioMode stretchScreen;

        QLabel *cursorAddress;
        QLabel *syslock;
        QLabel *insMode;

        // Session name
        QString sessionName;

        int blinkSpeed;
        bool blink;

        QTimer *blinker;
        QTimer *cursorBlinker;

};

#endif // TERMINAL_H
