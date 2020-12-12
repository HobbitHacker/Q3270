#include "ui_mainwindow.h"
#include "mainwindow.h"

//NOTE: SocketConnect will need to be created multiple times for multi-session support.

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), c(new(SocketConnection)), ui(new(Ui::MainWindow))
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationDomain("styles.homeip.net");
    QCoreApplication::setApplicationName("Q3270");
    QCoreApplication::setOrganizationName("andyWare");

    applicationSettings = new QSettings();

    cursorAddress = new QLabel("0,0");
    syslock = new QLabel(" ");
    insMode = new QLabel(" ");

    statusBar()->addPermanentWidget(syslock, 50);
    statusBar()->addPermanentWidget(insMode, 50);
    statusBar()->addPermanentWidget(cursorAddress, 50);

    //TODO will need a new Terminal() for each tab
    t = new Terminal();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupDisplay()
{
    connect(c, &SocketConnection::dataStreamComplete, this, &MainWindow::processDataStream);
}


void MainWindow::processDataStream(Buffer *b)
{
    datastream->processStream(b);
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

    if (applicationSettings->contains("terminal/model"))
    {
        t->setType(applicationSettings->value("terminal/model").toString());
        t->setSize(applicationSettings->value("terminal/width").toInt(), applicationSettings->value("terminal/height").toInt());
        t->setBlink(applicationSettings->value("terminal/cursorblink").toBool());
        t->setBlinkSpeed(applicationSettings->value("terminal/cursorblinkspeed").toInt());
    }

    if (applicationSettings->contains("font/scale"))
    {
        (applicationSettings->value("font/scale").toString() == "true") ? t->setScaleFont(true) : t->setScaleFont(false);
    }

    ui->verticalLayout->addWidget(t);

    gs = new QGraphicsScene();
    t->setScene(gs);

    datastream = new ProcessDataStream(gs, t);

    if(applicationSettings->contains("font/name"))
    {
        QFontDatabase *fd = new QFontDatabase();
        QFont f = fd->font(applicationSettings->value("font/name").toString(), applicationSettings->value("font/style").toString(), applicationSettings->value("font/size").toInt());
        datastream->setFont(f);
    }
    connect(datastream, &ProcessDataStream::cursorMoved, this, &MainWindow::showCursorAddress);

    Keyboard *kbd = new Keyboard(datastream);

    connect(kbd, &Keyboard::setLock, this, &MainWindow::setIndicators);
    connect(kbd, &Keyboard::saveKeyboardMapping, this, &MainWindow::setSetting);

    kbd->setMap();

    t->installEventFilter(kbd);

    //TODO most-recently-used list and dialog for connect
//    QHostInfo hi = QHostInfo::fromName("127.0.0.1");
    QHostInfo hi = QHostInfo::fromName("192.168.200.1");

//    c->connectMainframe(hi.addresses().first(), 3271, d, t);
    c->connectMainframe(hi.addresses().first(), 23,datastream,t);

    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setDisabled(true);
    ui->actionSet_Font->setEnabled(true);

}

void MainWindow::menuDisconnect()
{
    c->disconnectMainframe();

    ui->verticalLayout->removeWidget(t);

    delete datastream;
    gs->clear();
    delete gs;

    delete t;

    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
    ui->actionSet_Font->setDisabled(true);
}

void MainWindow::menuSetFont()
{
    FontSelection *fs;

    if (applicationSettings->contains("font/name"))
    {
        fs = new FontSelection(this,applicationSettings->value("font/name").toString(),applicationSettings->value("font/style").toString(),applicationSettings->value("font/size").toInt());
    }
    else
    {
        fs = new FontSelection(this,"ibm3270","Regular",8);
    }

    connect(fs, &FontSelection::setFont, this, &MainWindow::setSetting);

    if (fs->exec() == QDialog::Accepted)
    {
        datastream->setFont(fs->getFont());
        t->setScaleFont(fs->getScaling());
        ui->verticalLayout->update();
    }

    delete fs;
}

void MainWindow::menuTerminalSettings()
{
    Settings *s = new Settings(this, t);

    if (s->exec() == QDialog::Accepted)
    {
        setSetting("terminal/model", t->name());
        setSetting("terminal/height",QString::number(t->terminalHeight()));
        setSetting("terminal/width", QString::number(t->terminalWidth()));
        setSetting("terminal/cursorblink", QString::number(t->getBlink()));
        setSetting("terminal/cursorblinkspeed", QString::number(t->getBlinkSpeed()));
    }

    fflush(stdout);
}

void MainWindow::menuQuit()
{
    QApplication::quit();
}

void MainWindow::setSetting(QString key, QString value)
{
    applicationSettings->setValue(key, value);
    printf("Saving %s as %s\n", key.toLatin1().data(), value.toLatin1().data());
    fflush(stdout);
}
