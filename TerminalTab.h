#ifndef TERMINALTAB_H
#define TERMINALTAB_H

#include "ProcessDataStream.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "ColourTheme.h"
#include "CodePage.h"
#include "ActiveSettings.h"

#include <QSettings>
#include <QHostInfo>
#include <QGraphicsSimpleTextItem>
#include <QVBoxLayout>
#include <QMenuBar>

class TerminalTab : public QWidget
{
    Q_OBJECT

    public:

        TerminalTab(QVBoxLayout *v, ActiveSettings *activeSettings, CodePage &cp, Keyboard &kb, ColourTheme &cs, QString sessionName);
        ~TerminalTab();

        void openConnection(QString host, int port, QString luName);
        void openConnection(QString address);
        void openConnection(QSettings& s);

        int terminalWidth(bool alternate);
        int terminalHeight(bool alternate);
        int gridWidth(bool alternate);
        int gridHeight(bool alternate);

        void setBlink(bool blink);
        void setBlinkSpeed(int speed);

        void setFont();
        void setScreenStretch(bool scale);

        void setKeyboardTheme(QString themeName);
        
        // Return current session name
        inline QString getSessionName()    { return sessionName; };
        inline void    setSessionName(QString sessionName) { this->sessionName = sessionName; };

        DisplayScreen *setAlternateScreen(bool alt);

        void fit();

    signals:

        void connectionEstablished();
        void disconnected();
        void windowClosed(TerminalTab *t);

    public slots:

        void connected();
        void closeConnection();
        void closeEvent(QCloseEvent *closeEvent);
        void setCurrentFont(QFont f);
        void rulerStyle(int r);
        void rulerChanged(bool on);
        void changeCodePage();

        // Set themes by name
        void setColourTheme(QString themeName);

        void blinkText();
        void blinkCursor();

    private:

        void connectSession(QString host, int port, QString luName);
        void stopTimers();

        Keyboard &kbd;
        ColourTheme &colourtheme;
        CodePage &cp;

        QGraphicsView *view;

        QGraphicsScene *notConnectedScene;
        QGraphicsScene *primary;
        QGraphicsScene *alternate;

        DisplayScreen *primaryScreen;
        DisplayScreen *alternateScreen;
        DisplayScreen *current;

        ProcessDataStream *datastream;
        SocketConnection *socket;

        ActiveSettings *activeSettings;

        bool sessionConnected;

        Qt::AspectRatioMode stretchScreen;

        QLabel *cursorAddress;
        QLabel *syslock;
        QLabel *insMode;

        // Session name
        QString sessionName;

        int blinkSpeed;
        bool blink;

        QTimer *blinker;
        QTimer *cursorBlinker;

};

#endif // TERMINALTAB_H
