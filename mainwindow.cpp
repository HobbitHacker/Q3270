#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), c(new(SocketConnection)), ui(new(Ui::MainWindow))
{
    ui->setupUi(this);
    t = new Terminal();

    //    ui->statusBar().addPermanentWidget(new QLabel("C:000 R:000"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupDisplay()
{

//    QAction* connectAction = new QAction(this);
//    QAction* setFontAction = new QAction(this);

//    connectAction->setText("Connect");
    connect(c, &SocketConnection::dataStreamComplete, this, &MainWindow::processDataStream);

}


void MainWindow::processDataStream(Buffer *b)
{
    d->processStream(b);
	fflush(stdout);
}

void MainWindow::menuConnect()
{
//    QHostInfo hi = QHostInfo::fromName("fandezhi.efglobe.com");
//    c->connectMainframe(hi.addresses().first(), 23, d);

    display = new DisplayView();
    ui->verticalLayout->addWidget(display);

    gs = new QGraphicsScene();
    display->setScene(gs);

    d = new DisplayDataStream(gs, display, t);

    Keyboard *kbd = new Keyboard(d, c);
    display->installEventFilter(kbd);

    QHostInfo hi = QHostInfo::fromName("127.0.0.1");
    c->connectMainframe(hi.addresses().first(), 3271, d, t);

}

void MainWindow::menuSetFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont("ibm3270", 12), this);
    if (ok)
    {
        d->screen->setFont(font);
    }
}

void MainWindow::menuTerminalSettings()
{
    Settings *s = new Settings(this, t);
    bool ok = s->exec();
    if (ok == QDialog::Accepted)
    {
        printf("Yay, new settings");
    }
    else
    {
        printf("Nope, as you were");
    }
    fflush(stdout);
}

void MainWindow::menuQuit()
{

}
