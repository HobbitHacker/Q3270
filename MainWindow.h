#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QAction>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QTextDocument>
#include <QHostAddress>
#include <QTextEdit>
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QFontDialog>
#include <QSettings>
#include <QWidgetAction>
#include <QLabel>

#include "SocketConnection.h"
#include "ProcessDataStream.h"
#include "Keyboard.h"
#include "TerminalView.h"
#include "Settings.h"
#include "Q3270.h"
#include "TerminalTab.h"
#include "Host.h"
#include "ColourTheme.h"
#include "KeyboardTheme.h"
#include "SessionManagement.h"

QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }

QT_END_NAMESPACE

#define MRU_COUNT 10

class MainWindow : public QMainWindow
{

  Q_OBJECT
  
  public:

      struct Session {
          MainWindow *mw;
          QString session;
      };

      MainWindow(MainWindow::Session  = { nullptr, ""} );
      ~MainWindow();

  public slots:

      void closeEvent(QCloseEvent *c);
      void updateMRUlist(QString address);

  private slots:

      //File menu
      void menuNew();
      void menuDuplicate();
      void mruConnect();
      void menuSaveSession();
      void menuOpenSession();
      void menuManageSessions();
      void menuQuit();

      // Session menu
      void menuConnect();
      void menuReconnect();
      void menuDisconnect();
      void menuSessionPreferences();

      // Themes menu
      void menuKeyboardTheme();
      void menuColourTheme();

      // The Help->About dialog
      void menuAbout();

      // Triggered by windows being activated
      void updateMenuEntries();

  private:

      TerminalTab *terminal;
      Host *connectHost;
      ColourTheme *colourTheme;
      KeyboardTheme *keyboardTheme;
      SessionManagement *sm;

      int maxMruCount;

      QActionGroup *sessionGroup;

      Ui::MainWindow *ui;

      QStringList mruList;
};
 
#endif
