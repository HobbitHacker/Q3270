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
        void setConnected();
        void setDisconnected();

        DisplayScreen *setScreen(bool alt);

        DisplayScreen *primary;
        DisplayScreen *alternate;

        DisplayScreen *current;

        bool connected;
        bool scaleFont;

    private slots:

        void blinkText();
        void blinkCursor();

    private:

        bool stretch;

        int blinkSpeed;         // Cursor blink speed
        bool blink;             // Whether cursor blinks

        QTimer *blinker;
        QTimer *cursorBlinker;

};

#endif // TERMINALVIEW_H
