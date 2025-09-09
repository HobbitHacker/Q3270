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
#include <QMetaEnum>

#include "ActiveSettings.h"

/**
 * @brief
 * ActiveSettings represents the currently active settings. This is literally just the various flags
 * and values that would be written to the saved settings file.
 *
 * @details When a setting is changed (rather than just set back to the value it currently is), a singal
 *          is emitted. this avoids flagging a change when actually there wasn't one.
 *
 *          The constructor sets (my) factory setting defaults.
 */
ActiveSettings::ActiveSettings()
{
    rulerState = false;
    ruler = Q3270::CrossHair;

    cursorBlink = true;
    cursorBlinkSpeed = 4;
    cursorColourInherit = true;

    stretchScreen = true;
    backspaceStop = true;

    termFont = QFont("ibm3270", 8);
    
    termModel = Q3270_TERMINAL_MODEL2;
    termX = 80;
    termY = 24;

    codePage = "IBM-037";
    keyboardThemeName = "Factory";
    colourThemeName = "Factory";
    sessionName = "";
}

/**
 * @brief   ActiveSettings::setRulerState - hide or display the ruler
 * @param   rulerState - true to show the ruler, false to hide it.
 *
 * @details Change the state of the ruler display.
 */
void ActiveSettings::setRulerState(bool rulerState)
{
    if (rulerState != this->rulerState)
    {
        emit rulerChanged(rulerState);
    }

    this->rulerState = rulerState;
}

/**
 * @brief   ActiveSettings::setRulerStyle
 * @param   newStyle
 *
 * Value | Ruler Style
 * ----- | ------------
 * 0     | Crosshair
 * 1     | Vertical
 * 2     | Horizontal
 *
 * @sa Q3270.h
 *
 * @details Change the ruler style (crosshair, vertical, horizontal)
 */
void ActiveSettings::setRulerStyle(Q3270::RulerStyle newStyle)
{
    if (newStyle != this->ruler)
    {
        emit rulerStyleChanged(newStyle);
    }

    this->ruler = newStyle;
}

/**
 * @brief   ActiveSettings::setRulerStyleName
 * @param   newStyle - a string representing the keys in enum Q3270::RulerStyle
 *
 * @sa      Q3270.h
 *
 * @details Change the ruler style by name (CrossHair, Horizontal, Vertical). Defaults to CrossHair
 *          if an invalid one is passed.
 */
void ActiveSettings::setRulerStyleName(QString newStyle)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<Q3270::RulerStyle>();

    if (metaEnum.keyToValue(newStyle.toLatin1()) == -1)
    {
        setRulerStyle(Q3270::CrossHair);
    }
    else
    {
        setRulerStyle(Q3270::RulerStyle(metaEnum.keyToValue(newStyle.toLatin1())));
    }
}

/**
 * @brief   ActiveSettings::getRulerStyleName
 *
 * @sa      Q3270.h
 *
 * @details Return the name of the ruler style (CrossHair, Horizontal, Vertical).
 */
QString ActiveSettings::getRulerStyleName()
{
    QMetaEnum metaEnum = QMetaEnum::fromType<Q3270::RulerStyle>();

    return QString(metaEnum.valueToKey(this->ruler));
}

/**
 * @brief   ActiveSettings::setCursorBlink
 * @param   cursorBlink - true to blink the cursor, false to make it static.
 *
 * @details Change whether the cursor blinks or not.
 */
void ActiveSettings::setCursorBlink(bool cursorBlink)
{
    if (cursorBlink != this->cursorBlink)
    {
        emit cursorBlinkChanged(cursorBlink);
    }

    this->cursorBlink = cursorBlink;
}

/**
 * @brief   ActiveSettings::setCursorBlinkSpeed
 * @param   cursorBlinkSpeed - the speed of the cursor blink.
 *
 * @details Set the speed of cursor blink. This setting has no effect if the cursor
 *          isn't set to blink.
 */
void ActiveSettings::setCursorBlinkSpeed(int cursorBlinkSpeed)
{
    if (cursorBlinkSpeed != this->cursorBlinkSpeed)
    {
        emit cursorBlinkSpeedChanged(cursorBlinkSpeed);
    }

    this->cursorBlinkSpeed = cursorBlinkSpeed;
}

/**
 * @brief   ActiveSettings::setCursorColourInherit
 * @param   cursorInherit - true to inherit the colour of the cell, false for grey
 *
 * @details The cursor can inherit the colour of the underlying character cell, meaning that
 *          there is some visual feedback to the user of the colour of the field in which
 *          the cursor resides, or the cursor can be a simple grey colour.
 */
void ActiveSettings::setCursorColourInherit(bool cursorInherit)
{
    if (cursorInherit != this->cursorColourInherit)
    {
        emit cursorInheritChanged(cursorInherit);
    }
    this->cursorColourInherit = cursorInherit;
}

/**
 * @brief   ActiveSettings::setFont
 * @param   font
 * @details Change the font used for the display.
 */
void ActiveSettings::setFont(QFont font)
{
    if (font != this->termFont)
    {
        emit fontChanged(font);
    }
    this->termFont = font;
}

/**
 * @brief   ActiveSettings::setTerminal
 * @param   x     - the number of columns for the display
 * @param   y     - the number of rows for the display
 * @param   model - the model number
 *
 * @details Sets the size of the 3270 display (for the alternate screen). The primary screen is always 24x80
 *          (model 2). The setting is only changed when there is no active connection because part of the
 *          connection negotiation is to determine the screen size.
 *
 *          If one of the standard model types is used (2, 3, 4 or 5), the size is overridden and any
 *          size paramteres passed are ignored.
 *
 *          This routine is called when the Preferences dialog's OK button is clicked.
 */
void ActiveSettings::setTerminal(int x, int y, int model)
{
    // Prevent fixed size models overriding sizes
    switch(model)
    {
        case Q3270_TERMINAL_MODEL3:
            x = 80;
            y = 32;
            break;

        case Q3270_TERMINAL_MODEL4:
            x = 80;
            y = 43;
            break;

        case Q3270_TERMINAL_MODEL5:
            x = 132;
            y = 27;
            break;

        case Q3270_TERMINAL_DYNAMIC:
            break;

        default: // model 2 or unknown
            x = 80;
            y = 24;
            break;
    }

    if (x != termX || y != termY || model != termModel)
    {
        emit terminalModelChanged(x, y, model);
    }

    termX = x;
    termY = y;
    termModel = model;
}

/**
 * @brief   ActiveSettings::setTerminal
 * @param   x         - the number of columns for the display
 * @param   y         - the number of rows for the display
 * @param   modelName - the name of the model
 *
 * @details This routine is used to set the terminal size; it is called by Terminal when the
 *          configuration file is read in (so that the config file stores an English name rather
 *          than just a number)
 *
 *          Like setTerminal(), the size parameters are ignored if the model is a standard model.
 */
void ActiveSettings::setTerminal(int x, int y, QString modelName)
{
    if (modelName == "Model2")
    {
        setTerminal(80, 24, Q3270_TERMINAL_MODEL2);
        return;
    }
    if (modelName == "Model3")
    {
        setTerminal(80, 32, Q3270_TERMINAL_MODEL2);
        return;
    }
    if (modelName == "Model4")
    {
        setTerminal(80, 43, Q3270_TERMINAL_MODEL2);
        return;
    }
    if (modelName == "Model5")
    {
        setTerminal(132, 27, Q3270_TERMINAL_MODEL2);
        return;
    }

    setTerminal(x, y, Q3270_TERMINAL_DYNAMIC);
}

/**
 * @brief   ActiveSettings::getTerminalModelName
 * @return  The string form of the model name.
 *
 * @details This function returns the string format of the model name, which is stored in the config
 *          file.
 */
QString ActiveSettings::getTerminalModelName()
{
    switch(termModel)
    {
        case Q3270_TERMINAL_MODEL2:
            return "Model2";
        case Q3270_TERMINAL_MODEL3:
            return "Model3";
        case Q3270_TERMINAL_MODEL4:
            return "Model4";
        case Q3270_TERMINAL_MODEL5:
            return "Model5";
        default:
            return "Dynamic";
    }
}

/**
 * @brief   ActiveSettings::setCodePage
 * @param   codepage - the new codepage.
 *
 * @details Change the code page to the one specified.
 */
void ActiveSettings::setCodePage(QString codepage)
{
    if (this->codePage != codepage)
    {
        emit codePageChanged(codepage);
    }

    this->codePage = codepage;
}

/**
 * @brief   ActiveSettings::setKeyboardTheme
 * @param   keyboards         - The KeyboardTheme class
 * @param   keyboardThemeName - the name of the keyboard theme
 *
 * @details This setting holds the name of the keyboard theme. The KeyboardTheme class is needed
 *          because that's actually where the keyboard definitions are stored.
 */
void ActiveSettings::setKeyboardTheme(QString keyboardThemeName)
{
    if (this->keyboardThemeName != keyboardThemeName)
    {
        emit keyboardThemeChanged(keyboardThemeName);
    }

    this->keyboardThemeName = keyboardThemeName;
}

/**
 * @brief   ActiveSettings::setColourTheme
 * @param   colourthemeName - the name of the colour theme.
 *
 * @details The colour theme is set by name.
 */
void ActiveSettings::setColourTheme(QString colourthemeName)
{
    if (this->colourThemeName != colourthemeName)
    {
        emit colourThemeChanged(colourthemeName);
    }

    this->colourThemeName = colourthemeName;
}

/**
 * @brief   ActiveSettings::setHostAddress
 * @param   hostName
 * @param   hostPort
 * @param   hostLU
 *
 * @details Set the host address, port and LU name.
 */
void ActiveSettings::setHostAddress(QString hostName, int hostPort, QString hostLU)
{
    if (hostName != this->hostName || hostPort != this->hostPort || hostLU != this->hostLU)
    {
        emit hostChanged(hostName, hostPort, hostLU);
    }

    this->hostName = hostName;
    this->hostPort = hostPort;
    this->hostLU = hostLU;
}

/**
 * @brief ActiveSettings::setHostAddress
 * @param address - target host address.
 *
 * @details Set the host address. Host addresses can be of the form:
 *
 *   0700@localhost:3270
 *   console@1.2.3.4:23
 *   1.2.3.4:23
 *
 */
void ActiveSettings::setHostAddress(QString address)
{
    // Determine if the supplied address contains an '@' denoting the LU to be used.
    if (address.contains("@"))
    {
        hostLU = address.section("@", 0, 0);
        hostName = address.section("@", 1, 1).section(":", 0, 0);
        hostPort = address.section(":", 1, 1).toInt();
    } else
    {
        hostLU = "";
        hostName = address.section(":", 0, 0);
        hostPort = address.section(":", 1, 1).toInt();
    }

    emit hostChanged(hostName, hostPort, hostLU);
}

/**
 * @brief   ActiveSettings::getHostAddress
 * @return  The address of the host.
 *
 * @details The address of the host that Q3270 is connected to. This may include a port and LU.
 */
QString ActiveSettings::getHostAddress()
{
    QString address;

    if (hostLU != "")
    {
        address.append(hostLU + "@");
    }

    if (hostName != "")
    {
        address.append(hostName);

    }

    if (hostPort != 0)
    {
        address.append(":" + QString::number(hostPort));
    }

    return address;
}

/**
 * @brief   ActiveSettings::setBackspaceStop
 * @param   backspaceStop - whether pressing backspace will allow the user to move the cursor beyond the field
 *                        start.
 *
 * @details When backspace stop is enabled, the cursor will be prevented from continuing beyond the
 *          beginning of the start of the field. When backspace stop is disabled, and backspace is pressed,
 *          it will act like cursor left, rather than backspace.
 */
void ActiveSettings::setBackspaceStop(bool backspaceStop)
{
    if (this->backspaceStop != backspaceStop)
    {
        emit backspacesStopChanged(backspaceStop);
    }

    this->backspaceStop = backspaceStop;
}

/**
 * @brief   ActiveSettings::setStretchScreen
 * @param   stretch - true to ignore 4:3 ratio, and fill the window, false to enforce a 4:3 ratio.
 *
 * @details The screen can be made to fill the application window, so that the display is stretched to fill
 *          all space in the application window. If this setting is disabled, the characters form a fixed
 *          4:3 ratio size, and will have white space either side if the window is resized outside of a 4:3
 *          ratio.
 */
void ActiveSettings::setStretchScreen(bool stretch)
{
    if (this->stretchScreen != stretch)
    {
        emit stretchScreenChanged(stretch);
    }

    this->stretchScreen = stretch;
}

/**
 * @brief   ActiveSettings::setSecureMode
 * @param   stretch - true to use SSL, TLS
 *
 * @details Switches to SSL mode
 */
void ActiveSettings::setSecureMode(bool secureMode)
{
    if (this->secureMode != secureMode)
    {
        emit secureModeChanged(secureMode);
    }

    this->secureMode = secureMode;
}

/**
 * @brief   ActiveSettings::setVerifyCerts
 * @param   stretch - true to verify certificates of hosts
 *
 * @details Allows for self-signed certificates
 */
void ActiveSettings::setVerifyCerts(bool verify)
{
    if (this->verifyCerts != verify)
    {
        emit verifyCertsChanged(verify);
    }

    this->verifyCerts = verify;
}


void ActiveSettings::setSessionName(const QString &name)
{
    if (name != sessionName)
    {
        sessionName = name;
        emit sessionNameChanged(name);
    }

}
