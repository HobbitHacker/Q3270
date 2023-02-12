#include "TerminalTab.h"

TerminalTab::TerminalTab(QVBoxLayout *layout, ActiveSettings *activeSettings, CodePage *cp, Keyboard *kb, ColourTheme *cs, QString sessionName) :
    kbd(kb), colourtheme(cs), cp(cp), activeSettings(activeSettings), sessionName(sessionName)
{
    // Create terminal display and keyboard objects
    view = new TerminalView();

    connect(activeSettings, &ActiveSettings::rulerStyleChanged, this, &TerminalTab::rulerStyle);
    connect(activeSettings, &ActiveSettings::rulerChanged, this, &TerminalTab::rulerChanged);
    connect(activeSettings, &ActiveSettings::cursorBlinkChanged, view, &TerminalView::setBlink);
    connect(activeSettings, &ActiveSettings::cursorBlinkSpeedChanged, view, &TerminalView::setBlinkSpeed);
    connect(activeSettings, &ActiveSettings::fontChanged, this, &TerminalTab::setFont);
    connect(activeSettings, &ActiveSettings::codePageChanged, view, &TerminalView::changeCodePage);
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
    if (view->connected)
    {
        screen[0]->setFont(activeSettings->getFont());
        screen[1]->setFont(activeSettings->getFont());
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

void TerminalTab::setColourTheme(QString themeName)
{
    ColourTheme::Colours colours = colourtheme->getTheme(themeName);

    // Set colour theme by name; pass obtained theme to setColours()
    if (view->connected)
    {
        for (int i = 0; i < 2; i++)
        {
            screen[i]->setColourPalette(colours);
            screen[i]->resetColours();
        }
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
    if (view->connected)
    {
        screen[0]->rulerMode(on);
        screen[1]->rulerMode(on);
    }
}

void TerminalTab::rulerStyle(int rulerStyle)
{
    // Change ruler style to match settings
    if (view->connected)
    {
        screen[0]->setRulerStyle(rulerStyle);
        screen[1]->setRulerStyle(rulerStyle);
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
    view->setStretch(s.value("ScreenStretch").toBool());

    openConnection(s.value("Address").toString());
    setSessionName(s.group());

    // Update settings with address
    activeSettings->setHostAddress(s.value("Address").toString());

}

void TerminalTab::connectSession(QString host, int port, QString luName)
{
    setWindowTitle(windowTitle().append(" [").append(host).append("]"));

    screen[0] = new DisplayScreen(80, 24, cp);
    screen[1] = new DisplayScreen(activeSettings->getTerminalX(), activeSettings->getTerminalY(), cp);

    view->setScenes(screen[0], screen[1]);
    view->setAlternateScreen(false);

    datastream = new ProcessDataStream(view);
    socket = new SocketConnection(activeSettings->getTerminalModel());

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::connectionStarted, this, &TerminalTab::connected);
    connect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    kbd->setDataStream(datastream);

    //connect(settings, &PreferencesDialog::setStretch, view, &TerminalView::setStretch);

    //setColourTheme(colourTheme);

    for (int i = 0; i < 2; i++)
    {
        // screen[i]->setFontScaling(settings->getFontScaling());
        screen[i]->setFont(activeSettings->getFont());
        screen[i]->rulerMode(activeSettings->getRulerOn());
        screen[i]->setRulerStyle(activeSettings->getRulerStyle());
        screen[i]->setColourPalette(colourtheme->getTheme(activeSettings->getColourThemeName()));

        connect(datastream, &ProcessDataStream::cursorMoved, screen[i], &DisplayScreen::showStatusCursorPosition);

        connect(kbd, &Keyboard::setLock, screen[i], &DisplayScreen::setStatusXSystem);
        connect(kbd, &Keyboard::setInsert, screen[i], &DisplayScreen::setStatusInsert);

//        connect(settings, &PreferencesDialog::fontScalingChanged, screen[i], &DisplayScreen::setFontScaling);
        connect(activeSettings, &ActiveSettings::cursorInheritChanged, screen[i], &DisplayScreen::setCursorColour);

    }

    view->setBlink(activeSettings->getCursorBlink());
    view->setBlinkSpeed(activeSettings->getCursorBlinkSpeed());

    socket->connectMainframe(host, port, luName, datastream);

    kbd->setMap();

    view->installEventFilter(kbd);

    view->setConnected();
}

void TerminalTab::closeConnection()
{
    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    disconnect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    socket->disconnectMainframe();

    view->stopTimers();

    delete datastream;

    view->setScene(notConnectedScene);
    view->fitInView(notConnectedScene->itemsBoundingRect(), Qt::IgnoreAspectRatio);
    view->setDisconnected();

    delete screen[0];
    delete screen[1];

    // Menu "Connect" entry disable
    emit disconnected();
    kbd->setConnected(false);
}

void TerminalTab::connected()
{
    // Menu "Connect" entry enable
    emit connectionEstablished();
    kbd->setConnected(true);
}

void TerminalTab::closeEvent(QCloseEvent *closeEvent)
{
    if (view->connected)
    {
        closeConnection();
    }
    closeEvent->accept();
}
