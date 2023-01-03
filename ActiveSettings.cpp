#include "Q3270.h"

#include "ActiveSettings.h"

ActiveSettings::ActiveSettings()
{
    rulerOn = false;
    ruler = Q3270_RULER_CROSSHAIR;

    cursorBlink = true;
    cursorBlinkSpeed = 4;
    cursorInherit = true;
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

void ActiveSettings::setCursorInherit(bool cursorInherit)
{
    if (cursorInherit != this->cursorInherit)
    {
        emit cursorInheritChanged(cursorInherit);
    }
    this->cursorInherit = cursorInherit;
}
