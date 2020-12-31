#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include <QGraphicsView>
#include "DisplayScreen.h"

class TerminalView : public QGraphicsView
{
    Q_OBJECT

    public:
        TerminalView();

        void resizeEvent(QResizeEvent *r);

        void setScaleFont(bool scale);
        int  getBlinkSpeed();
        void setBlinkSpeed(int speed);
        bool getBlink();
        void setBlink(bool blink);
        void setScenes(DisplayScreen *primary, DisplayScreen *alternate);

        DisplayScreen *setScreen(bool alt);

        DisplayScreen *primary;
        DisplayScreen *alternate;

        DisplayScreen *current;

    private slots:

        void blinkText();
        void blinkCursor();

    private:

        bool resizeFont;

        int blinkSpeed;         // Cursor blink speed
        bool blink;             // Whether cursor blinks

        QTimer *blinker;
        QTimer *cursorBlinker;

};

#endif // TERMINALVIEW_H
