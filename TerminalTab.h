#ifndef TERMINALTAB_H
#define TERMINALTAB_H

#include "ProcessDataStream.h"
#include "TerminalView.h"
#include "Q3270.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "Settings.h"

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSettings>
#include <QHostInfo>
#include <QGraphicsSimpleTextItem>
#include <QVBoxLayout>
#include <QMenuBar>

#include <ColourTheme.h>

class TerminalTab : public QMdiSubWindow
{

    Q_OBJECT

    public:
        TerminalTab(ColourTheme *colours);
        ~TerminalTab();

        void openConnection(QString host, int port, QString luName);
        void openConnection(QString address);
        void connectSession();
        void closeConnection();

        int terminalWidth();
        int terminalHeight();
        char *name();

        void setWidth(int w);
        void setHeight(int h);

        void setType(QString type);
        void setType(int type);

        void setFont();
        void setScaleFont(bool scale);
        void setColours(ColourTheme::Colours colours);

        int getType();
        QString address();

        void showForm();

        TerminalView *view;

        QColor palette[8];

    signals:
        void connectionClosed();
        void windowClosed(TerminalTab *t);

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
        ColourTheme *colours;

        bool altScreen;

        QLabel *cursorAddress;
        QLabel *syslock;
        QLabel *insMode;

        // Local window menu
        QAction *actionConnect;
        QAction *actionReconnect;
        QAction *actionDisconnect;

        QString tabHost;
        int tabPort;
        QString tabLU;

        bool resizeFont;

};

#endif // TERMINALTAB_H
