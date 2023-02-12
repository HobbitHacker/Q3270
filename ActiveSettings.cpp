#include "Q3270.h"

#include "ActiveSettings.h"

ActiveSettings::ActiveSettings()
{
    rulerOn = false;
    ruler = Q3270_RULER_CROSSHAIR;

    cursorBlink = true;
    cursorBlinkSpeed = 4;
    cursorColourInherit = true;

    fontScaling = true;
    stretchScreen = true;
    backspaceStop = true;

    termFont = QFont("ibm3270", 8);
    
    termModel= 0;
    termX = 80;
    termY = 24;

    codePage = "IBM-037";
    keyboardThemeName = "Factory";
    colourThemeName = "Factory";

//    hostName = "";
//    hostLU = "";
//    hostPort = 0;
}

void ActiveSettings::setRulerOn(bool rulerOn)
{
    if (rulerOn != this->rulerOn)
    {
        emit rulerChanged(rulerOn);
    }

    this->rulerOn = rulerOn;
}

void ActiveSettings::setRulerStyle(int r)
{
    if (r != this->ruler)
    {
        emit rulerStyleChanged(r);
    }

    this->ruler = r;
}

void ActiveSettings::setCursorBlink(bool cursorBlink)
{
    if (cursorBlink != this->cursorBlink)
    {
        emit cursorBlinkChanged(cursorBlink);
    }

    this->cursorBlink = cursorBlink;
}

void ActiveSettings::setCursorBlinkSpeed(int cursorBlinkSpeed)
{
    if (cursorBlinkSpeed != this->cursorBlinkSpeed)
    {
        emit cursorBlinkSpeedChanged(cursorBlinkSpeed);
    }

    this->cursorBlinkSpeed = cursorBlinkSpeed;
}

void ActiveSettings::setCursorColourInherit(bool cursorInherit)
{
    if (cursorInherit != this->cursorColourInherit)
    {
        emit cursorInheritChanged(cursorInherit);
    }
    this->cursorColourInherit = cursorInherit;
}

void ActiveSettings::setFont(QFont font)
{
    if (font != this->termFont)
    {
        emit fontChanged(font);
    }
    this->termFont = font;
}

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

void ActiveSettings::setCodePage(QString codepage)
{
    if (this->codePage != codepage)
    {
        emit codePageChanged(codepage);
    }

    this->codePage = codepage;
}

void ActiveSettings::setKeyboardTheme(QString keyboardThemeName)
{
    if (this->keyboardThemeName != keyboardThemeName)
    {
        emit keyboardThemeChanged(keyboardThemeName);
    }

    this->keyboardThemeName = keyboardThemeName;
}

void ActiveSettings::setColourTheme(QString colourthemeName)
{
    if (this->colourThemeName != colourthemeName)
    {
        emit colourThemeChanged(colourthemeName);
    }

    this->colourThemeName = colourthemeName;
}
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

void ActiveSettings::setHostAddress(QString address)
{
    // Determine if the supplied address contains an '@' denoting the LU to be used.
    //
    // Addresses can be of the form:
    //
    //   0700@localhost:3270
    //   console@1.2.3.4:23
    //   1.2.3.4:23

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
        address.append(hostPort);
    }

    return address;
}

void ActiveSettings::setBackspaceStop(bool backspaceStop)
{
    if (this->backspaceStop != backspaceStop)
    {
        emit backspacesStopChanged(backspaceStop);
    }

    this->backspaceStop = backspaceStop;
}

void ActiveSettings::setStretchScreen(bool stretch)
{
    if (this->stretchScreen != stretch)
    {
        emit stretchScreenChanged(stretch);
    }

    this->stretchScreen = stretch;
}

void ActiveSettings::setFontScaling(bool scaling)
{
    if (this->fontScaling != scaling)
    {
        emit fontScalingChanged(scaling);
    }

    this->fontScaling = scaling;
}
