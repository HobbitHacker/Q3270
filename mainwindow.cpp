#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), c(new(SocketConnection)), ui(new(Ui::MainWindow))
{
    ui->setupUi(this);
    t = new Terminal();

    cursorAddress = new QLabel("0,0");
    syslock = new QLabel(" ");
    insMode = new QLabel(" ");

    statusBar()->addPermanentWidget(syslock, 50);
    statusBar()->addPermanentWidget(insMode, 50);
    statusBar()->addPermanentWidget(cursorAddress, 50);
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

void MainWindow::showCursorAddress(int x, int y)
{
    cursorAddress->setText(QString("%1,%2").arg(x + 1).arg(y + 1));
}

void MainWindow::setIndicators(Indicators ind)
{

    switch(ind) {
        case Indicators::InsertMode:
            insMode->setText(QString("^"));
            break;
        case Indicators::OvertypeMode:
            insMode->setText(QString(" "));
            break;
        case Indicators::SystemLock:
            syslock->setText(QString("X SYSTEM"));
            break;
        case Indicators::Unlocked:
            syslock->setText(QString(""));
            break;
        default:
            break;
    }
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
    connect(d, &DisplayDataStream::cursorMoved, this, &MainWindow::showCursorAddress);

    Keyboard *kbd = new Keyboard(d, c);

    connect(kbd, &Keyboard::setLock, this, &MainWindow::setIndicators);

    display->installEventFilter(kbd);

    QHostInfo hi = QHostInfo::fromName("127.0.0.1");
    c->connectMainframe(hi.addresses().first(), 3271, d, t);

    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setDisabled(true);

}

void MainWindow::menuDisconnect()
{

    c->disconnectMainframe();

    ui->verticalLayout->removeWidget(display);

    delete c;
    delete d;
    gs->clear();
    delete gs;

    delete display;

    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
}

void MainWindow::menuSetFont()
{

    // TODO: breaks if non connected
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
