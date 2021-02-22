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
#include <QStatusBar>
#include <QLabel>

class TerminalTab : public QMdiSubWindow
{

    Q_OBJECT

    public:
        TerminalTab();

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

    private slots:

        void closeEvent(QCloseEvent *closeEvent);
        void setCurrentFont(QFont f);

    private:

        QStatusBar *statusBar;
        QGraphicsScene *gs;

        ProcessDataStream *datastream;
        SocketConnection *socket;
        Keyboard *kbd;

        DisplayScreen *primary;
        DisplayScreen *alternate;

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
