#include "TerminalTab.h"

TerminalTab::TerminalTab(QVBoxLayout *layout, ColourTheme *colours)
{
    // Create Settings object
    settings = new Settings(colours);

    // Create terminal display and keyboard objects
    view = new TerminalView();
    kbd = new Keyboard(view);

    // Save ColourTheme object
    this->colours = colours;

    // Map settings signals to their target slots
    connect(settings, &Settings::coloursChanged, this, &TerminalTab::setColours);
    connect(settings, &Settings::fontChanged, this, &TerminalTab::setFont);
    connect(settings, &Settings::tempFontChange, this, &TerminalTab::setCurrentFont);
    connect(settings, &Settings::cursorBlinkChanged, view, &TerminalView::setBlink);
    connect(settings, &Settings::cursorBlinkSpeedChanged, view, &TerminalView::setBlinkSpeed);
    connect(settings, &Settings::newMap, kbd, &Keyboard::setNewMap);

    // Build "Not Connected" display
    gs = new QGraphicsScene();

    QGraphicsRectItem *mRect = new QGraphicsRectItem(0, 0, 640, 480);
    mRect->setBrush(QColor(Qt::black));
    mRect->setPen(QColor(Qt::black));

    gs->addItem(mRect);

    QGraphicsSimpleTextItem *ncMessage = new QGraphicsSimpleTextItem("Not Connected", mRect);

    ncMessage->setPen(QColor(Qt::white));

    QFont font("mono", 24);
    ncMessage->setFont(font);

    // Centre "Not Connected" based on font size. 640x480 halved, less the size of the font
    QFontMetrics fm(font);
    int xPos = 320 - fm.width(ncMessage->text()) / 2;
    int yPos = 240 - fm.height() / 2;
    ncMessage->setPos(xPos, yPos);

    gs->addItem(ncMessage);

    // Set "Not Connected" as the initially displayed scene
    view->setScene(gs);
    view->setStretch(settings->getStretch());

    // Add it to the display and finally show it
    layout->addWidget(view);

    view->show();
}

TerminalTab::~TerminalTab()
{
    delete kbd;
    delete settings;
    delete gs;
    delete view;
}

void TerminalTab::showForm()
{
    settings->setKeyboardMap(kbd->getMap());
    settings->showForm(view->connected);
}

void TerminalTab::setFont()
{
    if (view->connected)
    {
        screen[0]->setFont(settings->getFont());
        screen[1]->setFont(settings->getFont());
    }
}

void TerminalTab::setCurrentFont(QFont f)
{
    if (view->connected)
    {
        view->current->setFont(f);
    }
}

void TerminalTab::setScaleFont(bool scale)
{
    if (view->connected)
    {
        screen[0]->setFontScaling(scale);
        screen[1]->setFontScaling(scale);
    }
}

void TerminalTab::setColours(ColourTheme::Colours colours)
{
    if (view->connected)
    {
        for (int i = 0; i < 2; i++)
        {
            screen[i]->setColourPalette(colours);
            screen[i]->resetColours();
        }
    }
}

void TerminalTab::setColourTheme(QString themeName)
{
    colourTheme = themeName;

    // Set colour theme by name; pass obtained theme to setColours()
    if (view->connected)
    {
        setColours(colours->getTheme(themeName));
    }
}

void TerminalTab::openConnection(QString host, int port, QString luName)
{
    tabHost = host;
    tabPort = port;
    tabLU = luName;

    connectSession();
}

void TerminalTab::openConnection(QString address)
{
    if (address.contains("@"))
    {
       tabLU = address.section("@", 0, 0);
       tabHost = address.section("@", 1, 1).section(":", 0, 0);
       tabPort = address.section(":", 1, 1).toInt();
    }
    else
    {
        tabLU = "";
        tabHost = address.section(":", 0, 0);
        tabPort = address.section(":", 1, 1).toInt();
    }

    connectSession();

}

void TerminalTab::connectSession()
{
    setWindowTitle(windowTitle().append(" [").append(address()).append("]"));

    screen[0] = new DisplayScreen(80, 24, settings->getColours());
    screen[1] = new DisplayScreen(settings->getTermX(), settings->getTermY(), settings->getColours());

    view->setScenes(screen[0], screen[1]);
    view->setAlternateScreen(false);

    datastream = new ProcessDataStream(view);
    socket = new SocketConnection(settings->getTermName());

    kbd->setDataStream(datastream);

    connect(settings, &Settings::saveKeyboardSettings, kbd, &Keyboard::saveKeyboardSettings);
    connect(settings, &Settings::setStretch, view, &TerminalView::setStretch);

    setColourTheme(colourTheme);

    for (int i = 0; i < 2; i++)
    {
        screen[i]->setFontScaling(settings->getFontScaling());
        screen[i]->setFont(settings->getFont());

        connect(datastream, &ProcessDataStream::cursorMoved, screen[i], &DisplayScreen::showStatusCursorPosition);

        connect(kbd, &Keyboard::setLock, screen[i], &DisplayScreen::setStatusXSystem);
        connect(kbd, &Keyboard::setInsert, screen[i], &DisplayScreen::setStatusInsert);

        connect(settings, &Settings::fontScalingChanged, screen[i], &DisplayScreen::setFontScaling);
        connect(settings, &Settings::setCursorColour, screen[i], &DisplayScreen::setCursorColour);

    }

    view->setBlink(settings->getBlink());
    view->setBlinkSpeed(settings->getBlinkSpeed());

    QHostInfo hi = QHostInfo::fromName(tabHost);

    //TODO clazy warnings
    QList<QHostAddress> addresses = hi.addresses();
    socket->connectMainframe(addresses.first(), tabPort, tabLU, datastream);

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::disconnected3270, this, &TerminalTab::closeConnection);

    kbd->setMap();

    view->installEventFilter(kbd);

    view->setConnected();
/*
    actionConnect->setEnabled(false);
    actionReconnect->setEnabled(false);
    actionDisconnect->setEnabled(true);
    */
}

void TerminalTab::closeConnection()
{
    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    disconnect(socket, &SocketConnection::disconnected3270, this, &TerminalTab::closeConnection);

    socket->disconnectMainframe();

    view->stopTimers();

    delete datastream;

    view->setScene(gs);
    view->fitInView(gs->itemsBoundingRect(), Qt::IgnoreAspectRatio);
    view->setDisconnected();

    delete screen[0];
    delete screen[1];
/*
    actionConnect->setEnabled(true);
    actionReconnect->setEnabled(true);
    actionDisconnect->setEnabled(false);
*/
    emit connectionClosed();
}

QString TerminalTab::address()
{
    if (tabLU.isEmpty())
    {
        return tabHost + ":" + QString::number(tabPort);
    }
    else
    {
        return tabLU + "@" + tabHost + ":" + QString::number(tabPort);
    }
}

void TerminalTab::closeEvent(QCloseEvent *closeEvent)
{
    if (view->connected)
    {
        closeConnection();
    }
    closeEvent->accept();
}
