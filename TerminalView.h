#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include <QGraphicsView>
#include <QClipboard>
#include <QApplication>
#include <QMouseEvent>
#include "DisplayScreen.h"

class TerminalView : public QGraphicsView
{
    Q_OBJECT

    public:
        TerminalView();

        void resizeEvent(QResizeEvent *r);

        void mousePressEvent(QMouseEvent *mEvent);
        void mouseMoveEvent(QMouseEvent *mEvent);
        void mouseReleaseEvent(QMouseEvent *mEvent);

        void copyText();

        void setScaleFont(bool scale);
        void setBlinkSpeed(int speed);
        bool getBlink();
        bool getStretch();

        void setBlink(bool blink);
        void setScenes(DisplayScreen *primary, DisplayScreen *alternate);
        void setConnected();
        void setDisconnected();
        void clearSelection();

        void stopTimers();

        DisplayScreen *setAlternateScreen(bool alt);

        DisplayScreen *primary;
        DisplayScreen *alternate;

        DisplayScreen *current;

        bool connected;
        bool scaleFont;

        QPoint origin;
        QRubberBand *rubberBand;

    public slots:

        void setStretch(bool stretch);
        void changeCodePage();

    private slots:

        void blinkText();
        void blinkCursor();

    private:

        void fit();

        bool stretch;

        int blinkSpeed;         // Cursor blink speed
        bool blink;             // Whether cursor blinks

        QTimer *blinker;
        QTimer *cursorBlinker;
        QList <Glyph *>selection;

};

#endif // TERMINALVIEW_H
