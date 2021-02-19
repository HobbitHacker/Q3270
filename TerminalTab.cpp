#include "TerminalTab.h"

TerminalTab::TerminalTab()
{
    settings = new Settings(this->parentWidget());

    view = new TerminalView();

    gs = new QGraphicsScene();

    view->setScene(gs);

    this->setWidget(view);

}

void TerminalTab::showForm()
{
    settings->exec();
}

void TerminalTab::setFont(QFont f)
{
    if (view->connected)
    {
        primary->setFont(f);
        alternate->setFont(f);
    }
}

void TerminalTab::setScaleFont(bool scale)
{
    if (view->connected)
    {
        primary->setFontScaling(scale);
        alternate->setFontScaling(scale);
    }
}

void TerminalTab::setColours(QColor *colours)
{
    primary->setColourPalette(colours);
    alternate->setColourPalette(colours);

    primary->resetColours();
    alternate->resetColours();

    QSettings *set = new QSettings();
    set->beginWriteArray("colours");
    for (int i = 0; i < 8; i++)
    {
        set->setArrayIndex(i);
        set->setValue("colour", colours[i].name(QColor::HexRgb));
        palette[i] = colours[i];
    }
    set->endArray();
}

void TerminalTab::openConnection(QString host, int port, QString luName)
{
    tabHost = host;
    tabPort = port;
    tabLU = luName;

    connectSession();
}

void TerminalTab::connectSession()
{
    primary = new DisplayScreen(80, 24);
    alternate = new DisplayScreen(settings->getTermX(), settings->getTermY());

    primary->setColourPalette(settings->getColours());
    alternate->setColourPalette(settings->getColours());

    primary->resetColours();
    alternate->resetColours();

    view->setScenes(primary, alternate);
    view->setScreen(false);

    primary->setFontScaling(view->scaleFont);
    alternate->setFontScaling(view->scaleFont);

    primary->setFont(settings->getFont());
    alternate->setFont(settings->getFont());

    datastream = new ProcessDataStream(view);
    socket = new SocketConnection(settings->getTermName());

    connect(datastream, &ProcessDataStream::cursorMoved, primary, &DisplayScreen::showStatusCursorPosition);
    connect(datastream, &ProcessDataStream::cursorMoved, alternate, &DisplayScreen::showStatusCursorPosition);

    QHostInfo hi = QHostInfo::fromName(tabHost);

    //TODO clazy warnings
    QList<QHostAddress> addresses = hi.addresses();
    socket->connectMainframe(addresses.first(), tabPort, tabLU, datastream);

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::disconnected3270, this, &TerminalTab::closeConnection);

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
    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    disconnect(socket, &SocketConnection::disconnected3270, this, &TerminalTab::closeConnection);

//  disconnect(datastream, &ProcessDataStream::cursorMoved, primary, &DisplayScreen::showStatusCursorPosition);
//    disconnect(datastream, &ProcessDataStream::cursorMoved, alternate, &DisplayScreen::showStatusCursorPosition);

//    disconnect(kbd, &Keyboard::setLock, primary, &DisplayScreen::setStatusXSystem);
//    disconnect(kbd, &Keyboard::setLock, alternate, &DisplayScreen::setStatusXSystem);

//    disconnect(kbd, &Keyboard::setInsert, primary, &DisplayScreen::setStatusInsert);
//    disconnect(kbd, &Keyboard::setInsert, alternate, &DisplayScreen::setStatusInsert);

    socket->disconnectMainframe();

    view->stopTimers();

    delete socket;

    datastream->deleteLater();

    view->setScene(gs);
    view->setDisconnected();

    delete primary;
    delete alternate;
}

void TerminalTab::closeEvent(QCloseEvent *closeEvent)
{
    if (view->connected)
    {
        closeConnection();
    }
    QMdiSubWindow::closeEvent(closeEvent);
}
