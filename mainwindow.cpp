#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), c(new(SocketConnection)), ui(new(Ui::MainWindow))
{
    ui->setupUi(this);
//    ui->statusBar().addPermanentWidget(new QLabel("C:000 R:000"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupDisplay()
{
    display = new DisplayView();
    ui->verticalLayout->addWidget(display);

    gs = new QGraphicsScene();
    display->setScene(gs);

    QAction* connectAction = new QAction(this);
    connectAction->setText("Connect");

    connect(c, &SocketConnection::dataStreamComplete, this, &MainWindow::processDataStream);

    d = new DisplayDataStream(gs);

    Keyboard *kbd = new Keyboard(gs, d, c);
    gs->installEventFilter(kbd);
}


void MainWindow::processDataStream(Buffer *b)
{
    printf("Found something to process!\n");
    d->processStream(b);
	fflush(stdout);
}

void MainWindow::on_actionConnect_triggered(bool checked)
{
    QHostInfo hi = QHostInfo::fromName("fandezhi.efglobe.com");
    c->connectMainframe(hi.addresses().first(), 23, d);
//    QHostInfo hi = QHostInfo::fromName("127.0.0.1");
//    c->connectMainframe(hi.addresses().first(), 3270, d);
}
