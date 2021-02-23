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

#include "SocketConnection.h"
#include "ProcessDataStream.h"
#include "Keyboard.h"
#include "TerminalView.h"
#include "Settings.h"
#include "3270.h"
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
	
  private slots:
    //File menu
    void menuNew();
    void mruConnect();
    // Connect menu
    void menuConnect();
    void menuReconnect();
    void menuDisconnect();
    void menuQuit();
    // Settings menu
    void menuSetFont();
    void menuTerminalSettings();
    // Window Menu
    void menuTabbedView(bool tabView);

    // Triggered by windows being activated
    void updateMenuEntries();

  private:

    void updateMRUlist(QString address);

    Ui::MainWindow *ui;    

    QSettings *applicationSettings;

    Settings *settings;

    QMap<QMdiSubWindow *, TerminalTab *> sessions;

    QList<QString> mruList;
};
 
#endif
