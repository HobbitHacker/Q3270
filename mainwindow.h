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

#include "SocketConnection.h"
#include "DisplayDataStream.h"
#include "keyboard.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
	
  private slots:
    void on_actionConnect_triggered(bool checked);

  private:
	void setupActions();
    void processDataStream(Buffer *b);
	SocketConnection *c;
	DisplayDataStream *d;
    Ui::MainWindow *ui;    
    QTextEdit *te;
    QGraphicsScene *gs;
	
};
 
#endif
