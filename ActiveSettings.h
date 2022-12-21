#ifndef ACTIVESETTINGS_H
#define ACTIVESETTINGS_H

#include <QObject>

class ActiveSettings : public QObject
{
    Q_OBJECT

    public:

        ActiveSettings();

        bool getRulerOn()                          { return rulerOn; }
        void setRulerOn(bool rulerOn);

        bool getCursorBlink()                            { return cursorBlink; }
        void setCursorBlink(bool blink);

    signals:

        void rulerChanged(bool rulerOn);
        void cursorBlinkChanged(bool cursorBlink);

    private:

        bool rulerOn;                       // Whether crosshairs are shown
        bool cursorBlink;                   // Whether the cursor blinks

};

#endif // ACTIVESETTINGS_H
