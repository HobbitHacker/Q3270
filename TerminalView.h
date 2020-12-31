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
        void setBlinkSpeed(int speed);
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

        int blinkSpeed;

        QTimer *blinker;
        QTimer *cursorBlinker;

};

#endif // TERMINALVIEW_H
