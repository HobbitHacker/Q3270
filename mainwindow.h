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
#include "settings.h"
#include "FontSelection.h"
#include "3270.h"
#include "TerminalTab.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  public slots:
    void setSetting(QString k, QString v);
	
  private slots:
    // Connect menu
    void menuConnect();
    void menuDisconnect();
    void menuQuit();
    // Settings menu
    void menuSetFont();
    void menuTerminalSettings();

  private:
	void setupActions(); 

    bool xSystem;
    bool xClock;

    TerminalTab *t;
    Ui::MainWindow *ui;    
    QTextEdit *te;

    QSettings *applicationSettings;

    struct {
            int termType;
            int termX, termY;
    } settings;
};
 
#endif
