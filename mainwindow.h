#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QAction>
#include <QWidget>

#include "SocketConnection.h"
#include "DisplayDataStream.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
	
	virtual void paintEvent(QPaintEvent *qpainter);
	
  public slots:
	void makeConnection();
	
  private:
	void setupActions();
	void processDataStream();
	SocketConnection *c;
	DisplayDataStream *d;
    Ui::MainWindow *ui;
	
};
 
#endif
