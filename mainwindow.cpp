#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), c(new(SocketConnection)), ui(new(Ui::MainWindow))
{
    QAction* connectAction = new QAction(this);
    connectAction->setText("Connect");

    connect(c, &SocketConnection::dataStreamComplete, this, &MainWindow::processDataStream);

    gs = new QGraphicsScene();

    ui->setupUi(this);

    ui->graphicsView->setScene(gs);
    d = new DisplayDataStream(gs);

    Keyboard *kbd = new Keyboard(gs, d, c);
    gs->installEventFilter(kbd);

    statusBar()->addPermanentWidget(new QLabel("C:000 R:000"));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::processDataStream()
{
    printf("Found something to process!\n");
    d->processStream();
	fflush(stdout);
}

void MainWindow::on_actionConnect_triggered(bool checked)
{
    c->connectMainframe(QHostAddress("127.0.0.1"), 3270, d);
}
