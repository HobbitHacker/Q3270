#include "TerminalTab.h"

TerminalTab::TerminalTab(QVBoxLayout *layout, PreferencesDialog *settings, ActiveSettings *activeSettings, ColourTheme *colours, KeyboardTheme *keyboards, CodePage *cp, QString sessionName)
{
    // Create terminal display and keyboard objects
    view = new TerminalView();
    kbd = new Keyboard(view);

    // Save Themes and Settings objects
    this->activeSettings = activeSettings;
    this->colours = colours;
    this->keyboards = keyboards;
    this->settings = settings;
    this->cp = cp;

    // Save 'Session' name
    this->sessionName = sessionName;

    // Map settings signals to their target slots
    connect(settings, &PreferencesDialog::coloursChanged, this, &TerminalTab::setColours);
    connect(settings, &PreferencesDialog::fontChanged, this, &TerminalTab::setFont);
    connect(settings, &PreferencesDialog::tempFontChange, this, &TerminalTab::setCurrentFont);
    connect(settings, &PreferencesDialog::cursorBlinkSpeedChanged, view, &TerminalView::setBlinkSpeed);
    connect(settings, &PreferencesDialog::codePageChanged, view, &TerminalView::changeCodePage);
    connect(settings, &PreferencesDialog::setKeyboardTheme, kbd, &Keyboard::setTheme);
    connect(settings, &PreferencesDialog::rulerStyle, this, &TerminalTab::rulerStyle);

    connect(activeSettings, &ActiveSettings::rulerChanged, this, &TerminalTab::rulerChanged);
    connect(activeSettings, &ActiveSettings::cursorBlinkChanged, view, &TerminalView::setBlink);

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
    int xPos = 320 - fm.horizontalAdvance(ncMessage->text()) / 2;
    int yPos = 240 - fm.height() / 2;
    ncMessage->setPos(xPos, yPos);

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

void TerminalTab::setKeyboardTheme(QString themeName)
{
    keyboardTheme = themeName;

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

void TerminalTab::rulerStyle(DisplayScreen::RulerStyle rulerStyle)
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

void TerminalTab::connectSession(QString host, int port, QString luName)
{
    setWindowTitle(windowTitle().append(" [").append(host).append("]"));

    screen[0] = new DisplayScreen(80, 24, settings->getColours(), settings->codePage());
    screen[1] = new DisplayScreen(settings->getTermX(), settings->getTermY(), settings->getColours(), settings->codePage());

    view->setScenes(screen[0], screen[1]);
    view->setAlternateScreen(false);

    datastream = new ProcessDataStream(view);
    socket = new SocketConnection(settings->getTermName());

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::connectionStarted, this, &TerminalTab::connected);
    connect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    kbd->setDataStream(datastream);

    connect(settings, &PreferencesDialog::setStretch, view, &TerminalView::setStretch);

    setColourTheme(colourTheme);

    for (int i = 0; i < 2; i++)
    {
        screen[i]->setFontScaling(settings->getFontScaling());
        screen[i]->setFont(settings->getFont());
        screen[i]->rulerMode(activeSettings->getRulerOn());
        screen[i]->setRulerStyle(settings->getRulerStyle());

        connect(datastream, &ProcessDataStream::cursorMoved, screen[i], &DisplayScreen::showStatusCursorPosition);

        connect(kbd, &Keyboard::setLock, screen[i], &DisplayScreen::setStatusXSystem);
        connect(kbd, &Keyboard::setInsert, screen[i], &DisplayScreen::setStatusInsert);

        connect(settings, &PreferencesDialog::fontScalingChanged, screen[i], &DisplayScreen::setFontScaling);
        connect(settings, &PreferencesDialog::setCursorColour, screen[i], &DisplayScreen::setCursorColour);

    }

    view->setBlink(activeSettings->getCursorBlink());
    view->setBlinkSpeed(settings->getBlinkSpeed());

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

    view->setScene(gs);
    view->fitInView(gs->itemsBoundingRect(), Qt::IgnoreAspectRatio);
    view->setDisconnected();

    delete screen[0];
    delete screen[1];

    emit disconnected();
}

void TerminalTab::connected()
{
    emit connectionEstablished();
}

void TerminalTab::closeEvent(QCloseEvent *closeEvent)
{
    if (view->connected)
    {
        closeConnection();
    }
    closeEvent->accept();
}
