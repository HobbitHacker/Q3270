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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define MRU_COUNT 10

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  public slots:
    void setSetting(QString k, QString v);
    void closeEvent(QCloseEvent *c);

  private slots:
    //File menu
    void menuNew();
    void mruConnect();
    void menuQuit();

    // Connect menu
    void menuConnect();
    void menuReconnect();
    void menuDisconnect();

    // Settings menu
    void menuTerminalSettings();

    // Window Menu
    void menuTabbedView(bool tabView);

    // Triggered by windows being activated
    void updateMenuEntries();

    // Triggered by selecting a different session from the window menu

  private:

    void updateMRUlist(QString address);
    TerminalTab *newTab();

    int maxMruCount;
    int subWindow;

    QActionGroup *sessionGroup;

    Ui::MainWindow *ui;    

    QSettings *applicationSettings;

    QMap<QMdiSubWindow *, TerminalTab *> sessions;

    QList<QString> mruList;
};
 
#endif
