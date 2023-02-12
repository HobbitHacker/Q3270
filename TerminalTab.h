#ifndef TERMINALTAB_H
#define TERMINALTAB_H

#include "ProcessDataStream.h"
#include "TerminalView.h"
#include "SocketConnection.h"
#include "Keyboard.h"
#include "PreferencesDialog.h"
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

        TerminalTab(QVBoxLayout *v, ActiveSettings *activeSettings, CodePage *cp, Keyboard *kb, ColourTheme *cs, QString sessionName);
        ~TerminalTab();

        void openConnection(QString host, int port, QString luName);
        void openConnection(QString address);
        void openConnection(QSettings& s);

        int terminalWidth();
        int terminalHeight();
        char *name();

        void setWidth(int w);
        void setHeight(int h);

        void setType(QString type);
        void setType(int type);

        void setFont();
        void setScaleFont(bool scale);

        void setKeyboardTheme(QString themeName);
        
        // Return current session name
        inline QString getSessionName()    { return sessionName; };
        inline void    setSessionName(QString sessionName) { this->sessionName = sessionName; };

        int getType();

        void showForm();

        TerminalView *view;

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

        // Set themes by name
        void setColourTheme(QString themeName);

    private:

        void connectSession(QString host, int port, QString luName);

        Keyboard *kbd;
        ColourTheme *colourtheme;

        CodePage *cp;

        QGraphicsScene *notConnectedScene;

        ProcessDataStream *datastream;
        SocketConnection *socket;

        DisplayScreen *screen[2];

        ActiveSettings *activeSettings;

        bool altScreen;

        QLabel *cursorAddress;
        QLabel *syslock;
        QLabel *insMode;

        // Session name
        QString sessionName;

        bool resizeFont;

};

#endif // TERMINALTAB_H
