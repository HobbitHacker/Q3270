#include "TerminalTab.h"

TerminalTab::TerminalTab(QVBoxLayout *layout, ActiveSettings *activeSettings, CodePage *cp, Keyboard *kb, ColourTheme *cs, QString sessionName) :
    kbd(kb), colourtheme(cs), cp(cp), activeSettings(activeSettings), sessionName(sessionName)
{
    // Create terminal display and keyboard objects
    view = new QGraphicsView();

    view->setBackgroundBrush(QBrush(Qt::black));

    connect(activeSettings, &ActiveSettings::rulerStyleChanged, this, &TerminalTab::rulerStyle);
    connect(activeSettings, &ActiveSettings::rulerChanged, this, &TerminalTab::rulerChanged);
    connect(activeSettings, &ActiveSettings::cursorBlinkChanged, this, &TerminalTab::setBlink);
    connect(activeSettings, &ActiveSettings::cursorBlinkSpeedChanged, this, &TerminalTab::setBlinkSpeed);
    connect(activeSettings, &ActiveSettings::fontChanged, this, &TerminalTab::setFont);
    connect(activeSettings, &ActiveSettings::codePageChanged, this, &TerminalTab::changeCodePage);
    connect(activeSettings, &ActiveSettings::keyboardThemeChanged, kbd, &Keyboard::setTheme);

    //TODO: Move setColourTheme to ColourTheme - as Keyboard::setTheme? Would need to pass DisplayScreen
    connect(activeSettings, &ActiveSettings::colourThemeChanged, this, &TerminalTab::setColourTheme);

    // Build "Not Connected" display
    notConnectedScene = new QGraphicsScene();

    QGraphicsRectItem *mRect = new QGraphicsRectItem(0, 0, 640, 480);
    mRect->setBrush(QColor(Qt::black));
    mRect->setPen(QColor(Qt::black));

    notConnectedScene->addItem(mRect);

    QGraphicsSimpleTextItem *ncMessage = new QGraphicsSimpleTextItem("Not Connected", mRect);

    ncMessage->setPen(QColor(Qt::white));

    QFont font("mono", 24);
    ncMessage->setFont(font);

    // Centre "Not Connected" based on font size. 640x480 halved, less the size of the font
    QFontMetrics fm(font);
    int xPos = 320 - fm.horizontalAdvance(ncMessage->text()) / 2;
    int yPos = 240 - fm.height() / 2;
    ncMessage->setPos(xPos, yPos);

    // Set "Not Connected" as the initially displayed scene
    view->setScene(notConnectedScene);
    // view->setStretch(settings->getStretch());

    // Add it to the display and finally show it
    layout->addWidget(view);

    view->show();

    // Set up primary and alternate scenes
    primary = new QGraphicsScene();
    alternate = new QGraphicsScene();

    // Blink timers
    blinker = new QTimer(this);
    cursorBlinker = new QTimer(this);
}

TerminalTab::~TerminalTab()
{
    delete kbd;
//    delete settings;
    delete notConnectedScene;
    delete view;
}

void TerminalTab::showForm()
{
    // settings->showForm(view->connected);
}

// FIXME: Make this a signal/slot mechanism
void TerminalTab::setFont()
{
    if (sessionConnected)
    {
        primaryScreen->setFont(activeSettings->getFont());
        alternateScreen->setFont(activeSettings->getFont());
    }
}

void TerminalTab::setCurrentFont(QFont f)
{
    if (sessionConnected)
    {
        current->setFont(f);
    }
}

void TerminalTab::setScaleFont(bool scale)
{
    if (sessionConnected)
    {
        primaryScreen->setFontScaling(scale);
        alternateScreen->setFontScaling(scale);
    }
}

void TerminalTab::setColourTheme(QString themeName)
{
    ColourTheme::Colours colours = colourtheme->getTheme(themeName);

    // Set colour theme by name; pass obtained theme to setColours()
    if (sessionConnected)
    {
        primaryScreen->setColourPalette(colours);
        primaryScreen->resetColours();

        alternateScreen->setColourPalette(colours);
        alternateScreen->resetColours();

    }
}

void TerminalTab::setKeyboardTheme(QString themeName)
{
    //keyboardTheme = themeName;

    // Set keyboard theme by name; pass obtained theme to setKeyboard()

}

void TerminalTab::rulerChanged(bool on)
{
    // Switch ruler on or off apporpriately
    if (sessionConnected)
    {
        primaryScreen->rulerMode(on);
        alternateScreen->rulerMode(on);
    }
}

void TerminalTab::rulerStyle(int rulerStyle)
{
    // Change ruler style to match settings
    if (sessionConnected)
    {
        primaryScreen->setRulerStyle(rulerStyle);
        alternateScreen->setRulerStyle(rulerStyle);
    }
}

void TerminalTab::openConnection(QString host, int port, QString luName)
{
    connectSession(host, port, luName);
}

void TerminalTab::openConnection(QString address)
{
    if (address.contains("@"))
    {
        connectSession(address.section("@", 1, 1).section(":", 0, 0),
                       address.section(":", 1, 1).toInt(),
                       address.section("@", 0, 0));


    }
    else
    {
        connectSession(address.section(":", 0, 0),
                       address.section(":", 1, 1).toInt(),
                       "");

    }
}

void TerminalTab::openConnection(QSettings& s)
{
    // Set themes
    setColourTheme(s.value("ColourTheme").toString());
    setKeyboardTheme(s.value("KeyboardTheme").toString());

    // Set terminal characteristics
    activeSettings->setTerminal(s.value("TerminalX").toInt(), s.value("TerminalY").toInt(), s.value("TerminalModel").toString());
    activeSettings->setCodePage(s.value("Codepage").toString());

    // Cursor settings
    activeSettings->setCursorBlink(s.value("CursorBlink").toBool());
    activeSettings->setCursorBlinkSpeed(s.value("CursorBlinkSpeed").toInt());
    activeSettings->setCursorColourInherit(s.value("CursorInheritColour").toBool());

    // Ruler
    activeSettings->setRulerOn(s.value("Ruler").toBool());
    activeSettings->setRulerStyle(s.value("RulerStyle").toInt());

    // Font settings
    QFont f;
    f.setFamily(s.value("Font").toString());
    f.setPointSize(s.value("FontSize").toInt());
    f.setStyleName(s.value("FontStyle").toString());

    activeSettings->setFont(f);

    // settings->setFontScaling(s.value("FontScaling").toBool());
//    view->setStretch(s.value("ScreenStretch").toBool());

    openConnection(s.value("Address").toString());
    setSessionName(s.group());

    // Update settings with address
    activeSettings->setHostAddress(s.value("Address").toString());

}

void TerminalTab::connectSession(QString host, int port, QString luName)
{
    setWindowTitle(windowTitle().append(" [").append(host).append("]"));

    primaryScreen = new DisplayScreen(80, 24, cp);
    alternateScreen = new DisplayScreen(activeSettings->getTerminalX(), activeSettings->getTerminalY(), cp);

    primary->addItem(primaryScreen);
    alternate->addItem(alternateScreen);

    altScreen = false;

    datastream = new ProcessDataStream(this);
    socket = new SocketConnection(activeSettings->getTerminalModel());

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::connectionStarted, this, &TerminalTab::connected);
    connect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    kbd->setDataStream(datastream);

    //connect(settings, &PreferencesDialog::setStretch, view, &TerminalView::setStretch);

    //setColourTheme(colourTheme);

    primaryScreen->setFont(activeSettings->getFont());
    primaryScreen->rulerMode(activeSettings->getRulerOn());
    primaryScreen->setRulerStyle(activeSettings->getRulerStyle());
    primaryScreen->setColourPalette(colourtheme->getTheme(activeSettings->getColourThemeName()));

    connect(datastream, &ProcessDataStream::cursorMoved, primaryScreen, &DisplayScreen::showStatusCursorPosition);
    connect(kbd, &Keyboard::setLock, primaryScreen, &DisplayScreen::setStatusXSystem);
    connect(kbd, &Keyboard::setInsert, primaryScreen, &DisplayScreen::setStatusInsert);
    connect(activeSettings, &ActiveSettings::cursorInheritChanged, primaryScreen, &DisplayScreen::setCursorColour);

    alternateScreen->setFont(activeSettings->getFont());
    alternateScreen->rulerMode(activeSettings->getRulerOn());
    alternateScreen->setRulerStyle(activeSettings->getRulerStyle());
    alternateScreen->setColourPalette(colourtheme->getTheme(activeSettings->getColourThemeName()));

    connect(datastream, &ProcessDataStream::cursorMoved, alternateScreen, &DisplayScreen::showStatusCursorPosition);
    connect(kbd, &Keyboard::setLock, alternateScreen, &DisplayScreen::setStatusXSystem);
    connect(kbd, &Keyboard::setInsert, alternateScreen, &DisplayScreen::setStatusInsert);
    connect(activeSettings, &ActiveSettings::cursorInheritChanged, alternateScreen, &DisplayScreen::setCursorColour);

    setBlink(activeSettings->getCursorBlink());
    setBlinkSpeed(activeSettings->getCursorBlinkSpeed());

    socket->connectMainframe(host, port, luName, datastream);

    kbd->setMap();

    view->installEventFilter(kbd);

    connected();
}

void TerminalTab::closeConnection()
{
    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    disconnect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    socket->disconnectMainframe();

    stopTimers();

    delete datastream;

    view->setScene(notConnectedScene);

    delete primaryScreen;
    delete alternateScreen;

    // Menu "Connect" entry disable
    emit disconnected();

    kbd->setConnected(false);

    sessionConnected = false;
    view->setInteractive(false);
}

void TerminalTab::connected()
{
    // Menu "Connect" entry enable
    emit connectionEstablished();
    kbd->setConnected(true);

    sessionConnected = true;
    view->setInteractive(true);
    fit();
}

void TerminalTab::closeEvent(QCloseEvent *closeEvent)
{
    if (sessionConnected)
    {
        closeConnection();
    }
    closeEvent->accept();
}

void TerminalTab::setBlink(bool blink)
{
    this->blink = blink;
    if (!blink)
    {
        cursorBlinker->stop();
        if (sessionConnected)
        {
            current->showCursor();
        }
    }
    else
    {
        cursorBlinker->start();
    }
}

void TerminalTab::setBlinkSpeed(int speed)
{
    blinkSpeed = speed;
    cursorBlinker->stop();
    if (blinkSpeed > 0 && blink)
    {
        cursorBlinker->start((5 - blinkSpeed) * 250);
    }
}

void TerminalTab::changeCodePage()
{
    if (sessionConnected)
        current->setCodePage();
}

void TerminalTab::stopTimers()
{
    blinker->stop();
    cursorBlinker->stop();
    disconnect(blinker, &QTimer::timeout, current, &DisplayScreen::blink);
    disconnect(cursorBlinker, &QTimer::timeout, current, &DisplayScreen::cursorBlink);
}

void TerminalTab::blinkText()
{
    if (sessionConnected)
    {
        current->blink();
    }
}

void TerminalTab::blinkCursor()
{
    if (sessionConnected)
    {
        current->cursorBlink();
    }
}

DisplayScreen *TerminalTab::setAlternateScreen(bool alt)
{
    stopTimers();


    if (!alt)
    {
        view->setScene(primary);
        current = primaryScreen;
    }
    else
    {
        view->setScene(alternate);
        current = alternateScreen;
    }

    connect(blinker, &QTimer::timeout, current, &DisplayScreen::blink);
    blinker->start(1000);

    connect(cursorBlinker, &QTimer::timeout, current, &DisplayScreen::cursorBlink);
    setBlinkSpeed(blinkSpeed);

    fit();

    return current;
}

int TerminalTab::terminalWidth(bool alternate)
{
    if (!alternate)
    {
        return primaryScreen->width();
    }

    return alternateScreen->width();
}

int TerminalTab::terminalHeight(bool alternate)
{
    if (!alternate)
    {
        return primaryScreen->height();
    }

    return alternateScreen->height();
}

int TerminalTab::gridWidth(bool alternate)
{
    if (!alternate)
    {
        return primaryScreen->gridWidth();
    }

    return alternateScreen->gridWidth();
}

int TerminalTab::gridHeight(bool alternate)
{
    if (!alternate)
    {
        return primaryScreen->gridHeight();
    }

    return alternateScreen->gridHeight();
}

void TerminalTab::fit()
{
    // TODO: Is 640x480 always valid (ie, can we ditch the current-> in favour of 640x480)
    if (sessionConnected)
    {
//        view->fitInView(current->boundingRect(), Qt::IgnoreAspectRatio);
        view->fitInView(0, 0, 640, 500, Qt::IgnoreAspectRatio);
    }
    else
    {
        view->fitInView(QRectF(0, 0, 640, 480), Qt::IgnoreAspectRatio);
    }
}
