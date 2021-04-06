#ifndef TERMINALTAB_H
#define TERMINALTAB_H

#include "ProcessDataStream.h"
#include "TerminalView.h"
#include "3270.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "Settings.h"

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSettings>
#include <QHostInfo>
#include <QGraphicsSimpleTextItem>

class TerminalTab : public QMdiSubWindow
{

    Q_OBJECT

    public:
        TerminalTab();
        ~TerminalTab();

        void openConnection(QString host, int port, QString luName);
        void connectSession();
        void closeConnection();
        void connectionClosed();

        int terminalWidth();
        int terminalHeight();
        char *name();

        void setWidth(int w);
        void setHeight(int h);

        void setType(QString type);
        void setType(int type);

        void setFont();
        void setScaleFont(bool scale);
        void setColours(QColor colours[8]);

        int getType();

        void showForm();

        TerminalView *view;

        QColor palette[8];

    public slots:

        void activate(bool checked = false);

    private slots:

        void closeEvent(QCloseEvent *closeEvent);
        void setCurrentFont(QFont f);

    private:

        Keyboard *kbd;

        QGraphicsScene *gs;

        ProcessDataStream *datastream;
        SocketConnection *socket;

        DisplayScreen *screen[2];

        Settings *settings;

        bool altScreen;

        QLabel *cursorAddress;
        QLabel *syslock;
        QLabel *insMode;

        QString tabHost;
        int tabPort;
        QString tabLU;

        bool resizeFont;

};

#endif // TERMINALTAB_H
