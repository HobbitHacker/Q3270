#ifndef ACTIVESETTINGS_H
#define ACTIVESETTINGS_H

#include <QObject>

class ActiveSettings : public QObject
{
    Q_OBJECT

    public:

        ActiveSettings();

        bool getRulerOn()                                { return rulerOn; }
        void setRulerOn(bool rulerOn);

        int getRulerStyle()                              { return ruler; }
        void setRulerStyle(int r);

        bool getCursorBlink()                            { return cursorBlink; }
        void setCursorBlink(bool blink);

        int getCursorBlinkSpeed()                        { return cursorBlinkSpeed; }
        void setCursorBlinkSpeed(int blinkSpeed);

        bool getCursorInherit()                          { return cursorInherit; }
        void setCursorInherit(bool inherit);

    signals:

        void rulerChanged(bool rulerOn);
        void rulerStyleChanged(int r);

        void cursorBlinkChanged(bool cursorBlink);
        void cursorBlinkSpeedChanged(int b);

        void cursorInheritChanged(bool cursorInherit);

    private:

//        Q_ENUM(RulerStyle);

        bool rulerOn;                       // Whether crosshairs are shown
        int ruler;                          // Type of crosshairs, when shown

        bool cursorBlink;                   // Whether the cursor blinks
        int cursorBlinkSpeed;               // How fast the cursor blinks

        bool cursorInherit;                 // Whether the cursor inherits the colour of the character underneath
};

#endif // ACTIVESETTINGS_H
