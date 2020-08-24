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

#include "SocketConnection.h"
#include "DisplayDataStream.h"
#include "keyboard.h"
#include "DisplayView.h"
#include "Terminal.h"
#include "settings.h"

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
	
  private slots:
    // Connect menu
    void menuConnect();
    void menuQuit();
    // Settings menu
    void menuSetFont();
    void menuTerminalSettings();

  private:
	void setupActions();
    void processDataStream(Buffer *b);        

    Terminal *t;
	SocketConnection *c;
	DisplayDataStream *d;
    Ui::MainWindow *ui;    
    QTextEdit *te;
    QGraphicsScene *gs;
    DisplayView *display;

    struct {
            int termType;
            int termX, termY;
    } settings;
};
 
#endif
