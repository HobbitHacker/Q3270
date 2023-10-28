/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "TerminalTab.h"

TerminalTab::TerminalTab(QVBoxLayout *layout, ActiveSettings &activeSettings, CodePage &cp, Keyboard &kb, ColourTheme &cs, QString sessionName) :
    kbd(kb), colourtheme(cs), cp(cp), activeSettings(activeSettings), sessionName(sessionName)
{
    sessionConnected = false;

    // Create terminal display
    view = new QGraphicsView();

    view->setBackgroundBrush(QBrush(Qt::black));
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    // Connect signals for changes to settings so they can be reflected here
    connect(&activeSettings, &ActiveSettings::rulerStyleChanged, this, &TerminalTab::rulerStyle);
    connect(&activeSettings, &ActiveSettings::rulerChanged, this, &TerminalTab::rulerChanged);
    connect(&activeSettings, &ActiveSettings::cursorBlinkChanged, this, &TerminalTab::setBlink);
    connect(&activeSettings, &ActiveSettings::cursorBlinkSpeedChanged, this, &TerminalTab::setBlinkSpeed);
    connect(&activeSettings, &ActiveSettings::fontChanged, this, &TerminalTab::setFont);
    connect(&activeSettings, &ActiveSettings::codePageChanged, this, &TerminalTab::changeCodePage);
    connect(&activeSettings, &ActiveSettings::keyboardThemeChanged, &kbd, &Keyboard::setTheme);
    connect(&activeSettings, &ActiveSettings::stretchScreenChanged, this, &TerminalTab::setScreenStretch);

    //TODO: Move setColourTheme to ColourTheme - as Keyboard::setTheme? Would need to pass DisplayScreen
    connect(&activeSettings, &ActiveSettings::colourThemeChanged, this, &TerminalTab::setColourTheme);

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

    // Blink timers
    blinker = new QTimer(this);
    cursorBlinker = new QTimer(this);

    palette = colourtheme.getTheme(activeSettings.getColourThemeName());
}

TerminalTab::~TerminalTab()
{
    delete notConnectedScene;
    delete view;
}

void TerminalTab::setFont(QFont font)
{
    if (sessionConnected)
    {
        primaryScreen->setFont(font);
        alternateScreen->setFont(font);
    }
}

void TerminalTab::setCurrentFont(QFont f)
{
    if (sessionConnected)
    {
        current->setFont(f);
    }
}

void TerminalTab::setScreenStretch(bool stretch)
{
    stretchScreen = stretch ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio;
    fit();
}

void TerminalTab::setColourTheme(QString themeName)
{
    palette = colourtheme.getTheme(themeName);

    //FIXME: Might not be needed because of passing palette by reference to DisplayScreen
    // Set colour theme by name; pass obtained theme to setColours()
    if (sessionConnected)
    {
        primaryScreen->resetColours();
        alternateScreen->resetColours();

    }
}

//FIXME: What is this meant to be for?
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
    // Set terminal characteristics
    activeSettings.setTerminal(s.value("TerminalX").toInt(), s.value("TerminalY").toInt(), s.value("TerminalModel").toString());
    activeSettings.setCodePage(s.value("Codepage").toString());

    openConnection(s.value("Address").toString());

    // Cursor settings
    activeSettings.setCursorBlink(s.value("CursorBlink").toBool());
    activeSettings.setCursorBlinkSpeed(s.value("CursorBlinkSpeed").toInt());
    activeSettings.setCursorColourInherit(s.value("CursorInheritColour").toBool());

    // Ruler
    activeSettings.setRulerOn(s.value("Ruler").toBool());
    activeSettings.setRulerStyle(s.value("RulerStyle").toInt());

    // Font settings
    QFont f;
    f.setFamily(s.value("Font").toString());
    f.setPointSize(s.value("FontSize").toInt());
    f.setStyleName(s.value("FontStyle").toString());

    activeSettings.setFont(f);

    activeSettings.setStretchScreen(s.value("ScreenStretch").toBool());

    // Set themes
    setColourTheme(s.value("ColourTheme").toString());
    setKeyboardTheme(s.value("KeyboardTheme").toString());

    setSessionName(s.group());

    // Update settings with address
    activeSettings.setHostAddress(s.value("Address").toString());
}

void TerminalTab::connectSession(QString host, int port, QString luName)
{
    setWindowTitle(windowTitle().append(" [").append(host).append("]"));

    // Set up primary and alternate scenes
    primary = new QGraphicsScene();
    alternate = new QGraphicsScene();

    primaryScreen = new DisplayScreen(80, 24, cp, palette, primary);
    alternateScreen = new DisplayScreen(activeSettings.getTerminalX(), activeSettings.getTerminalY(), cp, palette, alternate);

    current = primaryScreen;
    view->setScene(primary);

    datastream = new ProcessDataStream(this);
    socket = new SocketConnection(activeSettings.getTerminalModel());

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::connectionStarted, this, &TerminalTab::connected);
    connect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    // Primary screen settings
    connect(&activeSettings, &ActiveSettings::cursorInheritChanged, primaryScreen, &DisplayScreen::setCursorColour);
    connect(datastream, &ProcessDataStream::cursorMoved, primaryScreen, &DisplayScreen::showStatusCursorPosition);

    connect(&kbd, &Keyboard::setLock, primaryScreen, &DisplayScreen::setStatusXSystem);
    connect(&kbd, &Keyboard::setInsert, primaryScreen, &DisplayScreen::setStatusInsert);

    primaryScreen->setFont(activeSettings.getFont());
    primaryScreen->rulerMode(activeSettings.getRulerOn());
    primaryScreen->setRulerStyle(activeSettings.getRulerStyle());

    // Alternate screen settings
    connect(&activeSettings, &ActiveSettings::cursorInheritChanged, alternateScreen, &DisplayScreen::setCursorColour);
    connect(datastream, &ProcessDataStream::cursorMoved, alternateScreen, &DisplayScreen::showStatusCursorPosition);

    connect(&kbd, &Keyboard::setLock, alternateScreen, &DisplayScreen::setStatusXSystem);
    connect(&kbd, &Keyboard::setInsert, alternateScreen, &DisplayScreen::setStatusInsert);

    alternateScreen->setFont(activeSettings.getFont());
    alternateScreen->rulerMode(activeSettings.getRulerOn());
    alternateScreen->setRulerStyle(activeSettings.getRulerStyle());

    // Status bar updates
    connect(datastream, &ProcessDataStream::keyboardUnlocked, &kbd, &Keyboard::unlockKeyboard);

    // Mouse click moves cursor
    connect(primaryScreen, &DisplayScreen::moveCursor, datastream, &ProcessDataStream::moveCursor);
    connect(alternateScreen, &DisplayScreen::moveCursor, datastream, &ProcessDataStream::moveCursor);

    // Keyboard inputs
    connect(&kbd, &Keyboard::key_moveCursor, datastream, &ProcessDataStream::moveCursor);
    connect(&kbd, &Keyboard::key_Backspace, datastream, &ProcessDataStream::backspace);
    connect(&kbd, &Keyboard::key_Tab, datastream, &ProcessDataStream::tab);
    connect(&kbd, &Keyboard::key_Backtab, datastream, &ProcessDataStream::backtab);
    connect(&kbd, &Keyboard::key_Attn, datastream, &ProcessDataStream::interruptProcess);
    connect(&kbd, &Keyboard::key_AID, datastream, &ProcessDataStream::processAID);
    connect(&kbd, &Keyboard::key_Home, datastream, &ProcessDataStream::home);
    connect(&kbd, &Keyboard::key_EraseEOF, datastream, &ProcessDataStream::eraseField);
    connect(&kbd, &Keyboard::key_Delete, datastream, &ProcessDataStream::deleteChar);
    connect(&kbd, &Keyboard::key_Newline, datastream, &ProcessDataStream::newline);
    connect(&kbd, &Keyboard::key_End, datastream, &ProcessDataStream::endline);
    connect(&kbd, &Keyboard::key_toggleRuler, datastream, &ProcessDataStream::toggleRuler);
    connect(&kbd, &Keyboard::key_Character, datastream, &ProcessDataStream::insertChar);

    connect(&kbd, &Keyboard::key_Copy, this, &TerminalTab::copyText);

    connect(&kbd, &Keyboard::key_showInfo, datastream, &ProcessDataStream::showInfo);

    setBlink(activeSettings.getCursorBlink());
    setBlinkSpeed(activeSettings.getCursorBlinkSpeed());

    socket->connectMainframe(host, port, luName, datastream);

    kbd.setMap();

    view->installEventFilter(&kbd);

    connected();
}

void TerminalTab::closeConnection()
{

    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    disconnect(socket, &SocketConnection::connectionStarted, this, &TerminalTab::connected);
    disconnect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    // Primary screen settings
    disconnect(&activeSettings, &ActiveSettings::cursorInheritChanged, primaryScreen, &DisplayScreen::setCursorColour);
    disconnect(datastream, &ProcessDataStream::cursorMoved, primaryScreen, &DisplayScreen::showStatusCursorPosition);

    disconnect(&kbd, &Keyboard::setLock, primaryScreen, &DisplayScreen::setStatusXSystem);
    disconnect(&kbd, &Keyboard::setInsert, primaryScreen, &DisplayScreen::setStatusInsert);

    // Alternate screen settings
    disconnect(&activeSettings, &ActiveSettings::cursorInheritChanged, alternateScreen, &DisplayScreen::setCursorColour);
    disconnect(datastream, &ProcessDataStream::cursorMoved, alternateScreen, &DisplayScreen::showStatusCursorPosition);

    disconnect(&kbd, &Keyboard::setLock, alternateScreen, &DisplayScreen::setStatusXSystem);
    disconnect(&kbd, &Keyboard::setInsert, alternateScreen, &DisplayScreen::setStatusInsert);

    // Status bar updates
    disconnect(datastream, &ProcessDataStream::keyboardUnlocked, &kbd, &Keyboard::unlockKeyboard);

    // Mouse click moves cursor
    disconnect(primaryScreen, &DisplayScreen::moveCursor, datastream, &ProcessDataStream::moveCursor);
    disconnect(alternateScreen, &DisplayScreen::moveCursor, datastream, &ProcessDataStream::moveCursor);

    // Keyboard inputs
    disconnect(&kbd, &Keyboard::key_moveCursor, datastream, &ProcessDataStream::moveCursor);
    disconnect(&kbd, &Keyboard::key_Backspace, datastream, &ProcessDataStream::backspace);
    disconnect(&kbd, &Keyboard::key_Tab, datastream, &ProcessDataStream::tab);
    disconnect(&kbd, &Keyboard::key_Backtab, datastream, &ProcessDataStream::backtab);
    disconnect(&kbd, &Keyboard::key_Attn, datastream, &ProcessDataStream::interruptProcess);
    disconnect(&kbd, &Keyboard::key_AID, datastream, &ProcessDataStream::processAID);
    disconnect(&kbd, &Keyboard::key_Home, datastream, &ProcessDataStream::home);
    disconnect(&kbd, &Keyboard::key_EraseEOF, datastream, &ProcessDataStream::eraseField);
    disconnect(&kbd, &Keyboard::key_Delete, datastream, &ProcessDataStream::deleteChar);
    disconnect(&kbd, &Keyboard::key_Newline, datastream, &ProcessDataStream::newline);
    disconnect(&kbd, &Keyboard::key_End, datastream, &ProcessDataStream::endline);
    disconnect(&kbd, &Keyboard::key_toggleRuler, datastream, &ProcessDataStream::toggleRuler);
    disconnect(&kbd, &Keyboard::key_Character, datastream, &ProcessDataStream::insertChar);

    disconnect(&kbd, &Keyboard::key_Copy, this, &TerminalTab::copyText);

    disconnect(&kbd, &Keyboard::key_showInfo, datastream, &ProcessDataStream::showInfo);

    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    disconnect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    socket->disconnectMainframe();

    stopTimers();

    delete datastream;

    view->setScene(notConnectedScene);

    delete primaryScreen;
    delete alternateScreen;

    delete primary;
    delete alternate;

    sessionConnected = false;

    // Menu "Connect" entry disable
    emit disconnected();

    kbd.setConnected(false);


    view->setInteractive(false);
}

void TerminalTab::connected()
{
    // Menu "Connect" entry enable
    emit connectionEstablished();
    kbd.setConnected(true);

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

void TerminalTab::fit()
{
    if (sessionConnected)
    {
        view->fitInView(0, 0, 640, 490, stretchScreen);
    }
    else
    {
        view->fitInView(QRectF(0, 0, 640, 480), Qt::IgnoreAspectRatio);
    }
}
