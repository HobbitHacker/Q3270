#include "ActiveSettings.h"

ActiveSettings::ActiveSettings()
{
    rulerOn = false;
    cursorBlink = false;
}

void ActiveSettings::setRulerOn(bool rulerOn)
{
    if (rulerOn != this->rulerOn)
    {
        emit rulerChanged(rulerOn);
    }

    this->rulerOn = rulerOn;
}

void ActiveSettings::setCursorBlink(bool cursorBlink)
{
    if (cursorBlink != this->cursorBlink)
    {
        emit cursorBlinkChanged(cursorBlink);
    }

    this->cursorBlink = cursorBlink;
}
