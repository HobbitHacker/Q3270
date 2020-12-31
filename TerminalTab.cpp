#include "TerminalTab.h"

TerminalTab::TerminalTab(QVBoxLayout *vbl, QSettings *applicationSettings)
{
    term = new TerminalView();

    statusBar = new QStatusBar(vbl->parentWidget());



/*    statusBar->addPermanentWidget(syslock, 50);
    statusBar->addPermanentWidget(insMode, 50);
    statusBar->addPermanentWidget(cursorAddress, 50);
*/
    this->vbl = vbl;
    vbl->addWidget(term);

    gs = new QGraphicsScene();

    term->setScene(gs);

    setType("IBM-3279-2-E");

    blink = true;
    blinkSpeed = 1000;

    if (applicationSettings->contains("terminal/model"))
    {
        setType(applicationSettings->value("terminal/model").toString());
        setSize(applicationSettings->value("terminal/width").toInt(), applicationSettings->value("terminal/height").toInt());
        blink = applicationSettings->value("terminal/cursorblink").toBool();
        blinkSpeed = applicationSettings->value("terminal/cursorblinkspeed").toInt();
        (applicationSettings->value("font/scale").toString() == "true") ? term->setScaleFont(true) : term->setScaleFont(false);
    }

    if (applicationSettings->contains("font/name"))
    {
        termFont.setFamily(applicationSettings->value("font/name").toString());
        termFont.setStyleName(applicationSettings->value("font/style").toString());
        termFont.setPointSize(applicationSettings->value("font/size").toInt());
    }
    else
    {
        termFont.setFamily("ibm32702");
        termFont.setStyleName("Regular");
        termFont.setPointSize(8);
    }

}

void TerminalTab::setType(int type)
{
    termType = type;
}

void TerminalTab::setType(QString type)
{
    for (int i = 0; i < 5; i++)
    {
        if (type == terms[i].term)
        {
            termType = i;
            return;
        }
    }

    termType = 0;
}

int TerminalTab::terminalWidth()
{
    return terms[termType].x;
}

int TerminalTab::terminalHeight()
{
    return terms[termType].y;
}

void TerminalTab::setSize(int x, int y)
{
    if (termType != 4)
    {
        return;
    }

    terms[4].x = x;
    terms[4].y = y;
}

int TerminalTab::getType()
{
    return termType;
}

char * TerminalTab::name()
{
    return terms[termType].term.toLatin1().data();
}

void TerminalTab::setBlink(bool b)
{
    blink = b;
}

void TerminalTab::setBlinkSpeed(int speed)
{
    if (blinkSpeed > 1000 || blinkSpeed < 0)
    {
        return;
    }

    blinkSpeed = speed;

    term->setBlinkSpeed(speed);
}

bool TerminalTab::getBlink()
{
    return blink;
}

int TerminalTab::getBlinkSpeed()
{
    return blinkSpeed;
}

void TerminalTab::setFont(QFont f)
{
    if (connected)
    {
        term->primary->setFont(f);
        term->alternate->setFont(f);
    }
    termFont = f;
}

void TerminalTab::openConnection()
{
    primary = new DisplayScreen(term->geometry().width(), term->geometry().height(), 80, 24);
    alternate = new DisplayScreen(term->geometry().width(), term->geometry().height(), terms[termType].x, terms[termType].y);

    term->setScenes(primary, alternate);

    primary->setFont(termFont);
    alternate->setFont(termFont);

    datastream = new ProcessDataStream(term);
    socket = new SocketConnection(terms[termType].term);

    connect(datastream, &ProcessDataStream::cursorMoved, this, &TerminalTab::showCursorAddress);

    //TODO most-recently-used list and dialog for connect
    QHostInfo hi = QHostInfo::fromName("127.0.0.1");
//    QHostInfo hi = QHostInfo::fromName("192.168.200.1");

    socket->connectMainframe(hi.addresses().first(), 3271, datastream);
//    c->connectMainframe(hi.addresses().first(), 23,datastream,t);

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::disconnected, this, &TerminalTab::closeConnection);

    Keyboard *kbd = new Keyboard(datastream);

    connect(kbd, &Keyboard::setLock, this, &TerminalTab::setIndicators);
//    connect(kbd, &Keyboard::saveKeyboardMapping, this, &MainWindow::setSetting);

    kbd->setMap();

    term->installEventFilter(kbd);

    connected = true;
}

void TerminalTab::showCursorAddress(int x, int y)
{
//    cursorAddress->setText(QString("%1,%2").arg(x + 1).arg(y + 1));
}

void TerminalTab::setIndicators(Indicators ind)
{
/*
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
    }*/
}

void TerminalTab::closeConnection()
{
    socket->disconnectMainframe();

    vbl->removeWidget(term);

    delete datastream;
    gs->clear();
    delete gs;

    delete term;

    connected = false;
}
