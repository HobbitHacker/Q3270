#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>
#include <QGraphicsView>

class Terminal : public QGraphicsView
{
    Q_OBJECT
    public:
        Terminal();

        void resizeEvent(QResizeEvent *r);

        int terminalWidth();
        int terminalHeight();
        char *name();

        void setWidth(int w);
        void setHeight(int h);

        void setType(QString type);
        void setType(int type);

        void setSize(int x, int y);
        void setBlink(bool b);
        void setBlinkSpeed(int s);
        void setScaleFont(bool scale);

        int getType();
        int getBlinkSpeed();
        bool getBlink();

    signals:

        void cursorBlinkChange();

    private:

        struct termTypes
        {
            QString term;
            int x, y;
        };

        termTypes terms[5] = {
            { "IBM-3279-2-E", 80, 24 },
            { "IBM-3279-3-E", 80, 32 },
            { "IBM-3279-4-E", 80, 43 },
            { "IBM-3279-5-E", 132, 27 },
            { "IBM-DYNAMIC", 0, 0}
        };

        int termType;
        bool blink;
        int blinkSpeed;
        bool resizeFont;

};

#endif // TERMINAL_H
