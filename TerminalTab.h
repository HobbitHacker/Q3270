#ifndef TERMINALTAB_H
#define TERMINALTAB_H

#include "ProcessDataStream.h"
#include "TerminalView.h"
#include "3270.h"
#include "SocketConnection.h"
#include "Keyboard.h"

#include <QVBoxLayout>
#include <QSettings>
#include <QStatusBar>

class TerminalTab : public QObject
{

    Q_OBJECT

    public:
        TerminalTab(QVBoxLayout *vbl, QSettings *applicationSettings);

        void openConnection();
        void closeConnection();

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

        void setFont(QFont f);

        int getType();
        int getBlinkSpeed();
        bool getBlink();

        TerminalView *term;

    public slots:

        void showCursorAddress(int x, int y);
        void setIndicators(Indicators i);
//        void processDataStream(Buffer *b);

    private:

        void blinkText();
        void blinkCursor();

        QTimer *blinker;             /* Blinking timer */
        QTimer *cursorBlinker;       /* Cursor Blink */

        QVBoxLayout *vbl;

        QStatusBar *statusBar;
        QGraphicsScene *gs;

        ProcessDataStream *datastream;
        SocketConnection *socket;
        Keyboard *kbd;

        DisplayScreen *primary;
        DisplayScreen *alternate;

        bool altScreen;

        QLabel *cursorAddress;
        QLabel *syslock;
        QLabel *insMode;

        QFont termFont;

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

        bool connected;
        int termType;
        bool blink;
        int blinkSpeed;
        bool resizeFont;

};

#endif // TERMINALTAB_H
