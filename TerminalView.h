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
        void setScenes(DisplayScreen *primary, DisplayScreen *alternate);
        DisplayScreen *setScreen(bool alt);

        DisplayScreen *primary;
        DisplayScreen *alternate;

        DisplayScreen *current;

    private:

        bool resizeFont;

};

#endif // TERMINALVIEW_H
