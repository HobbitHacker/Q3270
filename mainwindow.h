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
#include "DisplayView.h"
#include "Terminal.h"
#include "settings.h"
#include "3270.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  public:
    MainWindow(QWidget *parent = nullptr);
    void setupDisplay();
    ~MainWindow();

  public slots:
    void showCursorAddress(int x, int y);
    void setIndicators(Indicators i);
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
    void processDataStream(Buffer *b);        

    bool xSystem;
    bool xClock;

    Terminal *t;
	SocketConnection *c;
    ProcessDataStream *d;
    Ui::MainWindow *ui;    
    QTextEdit *te;
    QGraphicsScene *gs;
    DisplayView *display;

    QSettings *applicationSettings;

    QLabel *cursorAddress;
    QLabel *syslock;
    QLabel *insMode;

    struct {
            int termType;
            int termX, termY;
    } settings;
};
 
#endif
