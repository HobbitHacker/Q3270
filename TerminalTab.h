#ifndef TERMINALTAB_H
#define TERMINALTAB_H

#include "ProcessDataStream.h"
#include "TerminalView.h"
#include "Q3270.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "Settings.h"
#include "ColourTheme.h"

#include <QSettings>
#include <QHostInfo>
#include <QGraphicsSimpleTextItem>
#include <QVBoxLayout>
#include <QMenuBar>

class TerminalTab : public QWidget
{
    Q_OBJECT

    public:
        TerminalTab(QVBoxLayout *v, ColourTheme *colours, KeyboardTheme *keyboards, QString sessionName);
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

        // Set themes by name
        void setColourTheme(QString themeName);
        void setKeyboardTheme(QString themeName);
        
        // Return current theme names
        inline QString getColourTheme()    { return colourTheme; };
        inline QString getKeyboardTheme()  { return keyboardTheme; };

        // Return current session name
        inline QString getSessionName()    { return sessionName; };
        inline void    setSessionName(QString sessionName) { this->sessionName = sessionName; };

        int getType();
        QString address();

        void showForm();

        TerminalView *view;

    signals:
        void connectionChanged();
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

        ColourTheme *colours;
        KeyboardTheme *keyboards;

        Settings *settings;

        bool altScreen;

        QLabel *cursorAddress;
        QLabel *syslock;
        QLabel *insMode;

        QString tabHost;
        int tabPort;
        QString tabLU;
        
        // Current themes
        QString colourTheme;
        QString keyboardTheme;

        // Session name
        QString sessionName;

        bool resizeFont;

};

#endif // TERMINALTAB_H
