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

#ifndef ACTIVESETTINGS_H
#define ACTIVESETTINGS_H

#include <QFont>
#include <QObject>

#include "Q3270.h"

class ActiveSettings : public QObject
{
    Q_OBJECT

    public:

        ActiveSettings();

        QString getHostName()                            { return hostName; }
        int getHostPort()                                { return hostPort; }
        QString getHostLU()                              { return hostLU; }
        QString getHostAddress();
        void setHostAddress(QString hostName, int port, QString hostLU);
        void setHostAddress(QString address);

        bool getRulerState()                             { return rulerState; }
        void setRulerState(bool rulerOn);

        Q3270::RulerStyle getRulerStyle()                { return ruler; }
        void setRulerStyle(Q3270::RulerStyle r);
        QString getRulerStyleName();
        void setRulerStyleName(QString s);

        bool getCursorBlink()                            { return cursorBlink; }
        void setCursorBlink(bool blink);

        int getCursorBlinkSpeed()                        { return cursorBlinkSpeed; }
        void setCursorBlinkSpeed(int blinkSpeed);

        bool getCursorColourInherit()                    { return cursorColourInherit; }
        void setCursorColourInherit(bool inherit);

        bool getStretchScreen()                          { return stretchScreen; }
        void setStretchScreen(bool stretch);

        bool getBackspaceStop()                          { return backspaceStop; }
        void setBackspaceStop(bool backspaceStop);

        bool getSecureMode()                             { return secureMode; }
        void setSecureMode(bool secureMode);

        bool getVerifyCerts()                           { return verifyCerts; }
        void setVerifyCerts(bool verifyCerts);

        QFont getFont()                                  { return termFont; }
        void setFont(QFont font);

        int getTerminalX()                               { return termX; }
        int getTerminalY()                               { return termY; }
        int getTerminalModel()                           { return termModel; }
        QString getTerminalModelName();

        void setTerminal(int x, int y, int model);
        void setTerminal(int x, int y, QString modelName);

        QString getCodePage()                            { return codePage; }
        void setCodePage(QString codePage);

        QString getKeyboardThemeName()                   { return keyboardThemeName; }
        void setKeyboardTheme(QString keyboardThemeName);

        QString getColourThemeName()                     { return colourThemeName; }
        void setColourTheme(QString colourThemeName);

    signals:

        void rulerChanged(bool rulerOn);
        void rulerStyleChanged(Q3270::RulerStyle r);

        void cursorBlinkChanged(bool cursorBlink);
        void cursorBlinkSpeedChanged(int b);

        void cursorInheritChanged(bool cursorInherit);

        void terminalModelChanged(int x, int y, int model);

        void fontChanged(QFont font);

        void codePageChanged(QString codepage);

        void keyboardThemeChanged(QString keyboardThemeName);
        void colourThemeChanged(QString colourThemeName);

        void hostChanged(QString hostName, int hostPport, QString hostLu);

        void stretchScreenChanged(bool stretchScreen);
        void backspacesStopChanged(bool backspaceStop);
        void cursorColourInheritChanged(bool cursorColourInherit);
        void fontScalingChanged(bool scaling);

        void secureModeChanged(bool secureMode);
        void verifyCertsChanged(bool verifyCerts);

    private:

//        Q_ENUM(RulerStyle);

        QFont termFont;

        // Terminal characteristics
        int termX;
        int termY;
        int termModel;
        QString termModelName;
        QString codePage;

        // Host address parts
        QString hostName;
        int     hostPort;
        QString hostLU;

        bool secureMode;
        bool verifyCerts;


        // Themes
        QString keyboardThemeName;
        QString colourThemeName;

        // Terminal behaviours
        bool stretchScreen;                 // Whether to stretch the 3270 screen to fit the window
        bool backspaceStop;                 // Whether backspace stops at the field start position
        bool cursorColourInherit;           // Whether the cursor colour matches the colour of the character underneath

        bool rulerState;                    // Whether crosshairs are shown
        Q3270::RulerStyle ruler;                          // Type of crosshairs, when shown

        bool cursorBlink;                   // Whether the cursor blinks
        int cursorBlinkSpeed;               // How fast the cursor blinks

};

#endif // ACTIVESETTINGS_H
