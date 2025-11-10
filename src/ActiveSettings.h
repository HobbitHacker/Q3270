/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
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

        QString getHostName() const                      { return hostName; }
        int getHostPort() const                          { return hostPort; }
        QString getHostLU() const                        { return hostLU; }
        QString getHostAddress() const;
        void setHostAddress(const QString &hostName, int port, const QString &hostLU);
        void setHostAddress(const QString &address);
        void applyUserHostChange(const QString &hostName, int port, const QString &hostLU);

        bool getRulerState() const                       { return rulerState; }
        void setRulerState(bool rulerOn);

        Q3270::RulerStyle getRulerStyle() const          { return ruler; }
        void setRulerStyle(Q3270::RulerStyle r);
        QString getRulerStyleName() const;
        void setRulerStyleName(const QString &s);

        bool getCursorBlink() const                      { return cursorBlink; }
        void setCursorBlink(bool blink);

        int getCursorBlinkSpeed() const                  { return cursorBlinkSpeed; }
        void setCursorBlinkSpeed(int blinkSpeed);

        bool getCursorColourInherit() const              { return cursorColourInherit; }
        void setCursorColourInherit(bool inherit);

        bool getStretchScreen() const                    { return stretchScreen; }
        void setStretchScreen(bool stretch);

        bool getBackspaceStop() const                    { return backspaceStop; }
        void setBackspaceStop(bool backspaceStop);

        bool getSecureMode() const                       { return secureMode; }
        void setSecureMode(bool secureMode);

        bool getVerifyCerts() const                      { return verifyCerts; }
        void setVerifyCerts(bool verifyCerts);

        QFont getFont() const                            { return termFont; }
        void setFont(const QFont &font);
        
        Q3270::FontTweak getTweak() const                { return tweaks; }
        void  setTweak(Q3270::FontTweak tweak);

        int getTerminalX() const                         { return termX; }
        int getTerminalY() const                         { return termY; }
        int getTerminalModel() const                     { return termModel; }
        QString getTerminalModelName() const;

        void setTerminal(int x, int y, int model);
        void setTerminal(int x, int y, const QString &modelName);

        QString getCodePage() const                      { return codePage; }
        void setCodePage(const QString &codePage);

        QString getKeyboardThemeName() const             { return keyboardThemeName; }
        void setKeyboardTheme(const QString &keyboardThemeName);

        QString getColourThemeName() const               { return colourThemeName; }
        void setColourTheme(const QString &colourThemeName);

        QString getSessionName() const                   { return sessionName; }
        void setSessionName(const QString &name);

        QString getDescription() const                   { return description; }
        void setDescription(const QString &description);

    signals:

        void rulerChanged(bool rulerOn);
        void rulerStyleChanged(Q3270::RulerStyle r);

        void cursorBlinkChanged(bool cursorBlink);
        void cursorBlinkSpeedChanged(int b);

        void cursorInheritChanged(bool cursorInherit);

        void terminalModelChanged(int x, int y, int model);

        void fontChanged(QFont font);
        void fontTweakChanged(Q3270::FontTweak t);

        void codePageChanged(QString codepage);

        void keyboardThemeChanged(const QString &keyboardThemeName);
        void colourThemeChanged(QString colourThemeName);

        void hostChanged(const QString &hostName, const int hostPort, const QString &hostLu);

        void stretchScreenChanged(bool stretchScreen);
        void backspacesStopChanged(bool backspaceStop);
        void cursorColourInheritChanged(bool cursorColourInherit);
        void fontScalingChanged(bool scaling);

        void secureModeChanged(bool secureMode);
        void verifyCertsChanged(bool verifyCerts);

        void sessionNameChanged(const QString name);

        void descriptionChanged(const QString description);

    private:

//        Q_ENUM(RulerStyle);

        QString sessionName;
        QString description;

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

        Q3270::FontTweak tweaks;
};

#endif // ACTIVESETTINGS_H
