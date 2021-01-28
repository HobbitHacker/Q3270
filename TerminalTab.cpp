#include "TerminalTab.h"

TerminalTab::TerminalTab(QSettings *applicationSettings)
{
    view = new TerminalView();

    gs = new QGraphicsScene();

    view->setScene(gs);

    this->setWidget(view);

    setType("IBM-3279-2-E");

    if (applicationSettings->contains("terminal/model"))
    {
        setType(applicationSettings->value("terminal/model").toString());
        setSize(applicationSettings->value("terminal/width").toInt(), applicationSettings->value("terminal/height").toInt());
        view->setBlink(applicationSettings->value("terminal/cursorblink").toBool());
        view->setBlinkSpeed(applicationSettings->value("terminal/cursorblinkspeed").toInt());
        (applicationSettings->value("font/scale").toString() == "true") ? view->setScaleFont(true) : view->setScaleFont(false);
    }

    if (applicationSettings->contains("font/name"))
    {
        termFont.setFamily(applicationSettings->value("font/name").toString());
        termFont.setStyleName(applicationSettings->value("font/style").toString());
        termFont.setPointSize(applicationSettings->value("font/size").toInt());
        (applicationSettings->value("font/scale").toInt() == 0) ? view->setScaleFont(false) : view->setScaleFont(true);
    }
    else
    {
        termFont.setFamily("ibm3270");
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
    char *ttype = terms[termType].term.toLatin1().data();

    return ttype;
}

void TerminalTab::setFont(QFont f)
{
    if (view->connected)
    {
        view->primary->setFont(f);
        view->alternate->setFont(f);
    }
    termFont = f;
}

void TerminalTab::setScaleFont(bool scale)
{
    if (view->connected)
    {
        primary->setFontScaling(scale);
        alternate->setFontScaling(scale);
    }
}

void TerminalTab::openConnection(QString host, int port, QString luName)
{
    primary = new DisplayScreen(80, 24);
    alternate = new DisplayScreen(terms[termType].x, terms[termType].y);

    view->setScenes(primary, alternate);
    view->setScreen(false);

    primary->setFontScaling(view->scaleFont);
    alternate->setFontScaling(view->scaleFont);

    primary->setFont(termFont);
    alternate->setFont(termFont);

    datastream = new ProcessDataStream(view);
    socket = new SocketConnection(terms[termType].term);

    connect(datastream, &ProcessDataStream::cursorMoved, primary, &DisplayScreen::showStatusCursorPosition);
    connect(datastream, &ProcessDataStream::cursorMoved, alternate, &DisplayScreen::showStatusCursorPosition);

    //TODO most-recently-used list and dialog for connect
    QHostInfo hi = QHostInfo::fromName(host);

    socket->connectMainframe(hi.addresses().first(), port, luName, datastream);

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::disconnected, this, &TerminalTab::closeConnection);

    Keyboard *kbd = new Keyboard(datastream, view);

    connect(kbd, &Keyboard::setLock, primary, &DisplayScreen::setStatusXSystem);
    connect(kbd, &Keyboard::setLock, alternate, &DisplayScreen::setStatusXSystem);

    connect(kbd, &Keyboard::setInsert, primary, &DisplayScreen::setStatusInsert);
    connect(kbd, &Keyboard::setInsert, alternate, &DisplayScreen::setStatusInsert);

    kbd->setMap();

    view->installEventFilter(kbd);

    view->setConnected();
}

void TerminalTab::closeConnection()
{

    socket->disconnectMainframe();

    delete socket;

    delete datastream;

    delete primary;
    delete alternate;

    view->setScene(gs);

    view->setDisconnected();
}
