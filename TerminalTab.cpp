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

/**
 * @brief   TerminalTab::TerminalTab - the terminal in the Qt window
 * @param   layout          - the central widget in which the QGraphicsView which shows the terminal
 * @param   activeSettings  - currently active settings
 * @param   cp              - shared CodePage object
 * @param   kb              - shared Keyboard object
 * @param   cs              - shared ColourTheme object
 * @param   sessionName     - the session name
 *
 * @details This is the controlling widget for the terminal. It comprises a QGraphicsView which is used to
 *          display the 3270 matrix and connects the Keyboard. It handles the opening and closing of
 *          connections (to show the "Not Connected" display) and switches between primary and alternate
 *          displays.
 *
 *          There are two displays built; the primary one is always 24x80, and the alternate is defined
 *          by the user, based on the Terminal Model type selected.
 */
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
    view->setInteractive(false);

    // Plug the keyboard in
    view->installEventFilter(&kbd);

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

    blinkSpeed = activeSettings.getCursorBlinkSpeed();

    palette = colourtheme.getTheme(activeSettings.getColourThemeName());
}

/**
 * @brief   TerminalTab::~TerminalTab - destructor
 *
 * @details Delete the objects obtained via new.
 */
TerminalTab::~TerminalTab()
{
    delete notConnectedScene;
    delete view;
}

/**
 * @brief   TerminalTab::setFont - change the font on the primary and alternate screens
 * @param   font - the chosen font
 *
 * @details Called when the user changes the font.
 */
void TerminalTab::setFont(QFont font)
{
    if (sessionConnected)
    {
        primaryScreen->setFont(font);
        alternateScreen->setFont(font);
    }
}

/**
 * @brief   TerminalTab::setCurrentFont - temporarily change the font on the current screen
 * @param   f - the chosen font
 *
 * @details Called to dynamically change the font on the currently displayed screen. Used during the
 *          Preferences dialog display to show the different fonts as they are selected by the user.
 *
 * @note    This processing is currently disabled.
 */
void TerminalTab::setCurrentFont(QFont f)
{
    if (sessionConnected)
    {
        current->setFont(f);
    }
}

/**
 * @brief   TerminalTab::setScreenStretch - toggle the screen stretch and fit the content
 * @param   stretch - true to stretch to fit the window, false to maintain 4:3 ratio
 *
 * @details Called when the user has switched from stretching the screen to fit the window or to
 *          maintain a 4:3 ratio.
 */
void TerminalTab::setScreenStretch(bool stretch)
{
    stretchScreen = stretch ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio;
    fit();
}

/**
 * @brief   TerminalTab::setColourTheme - switch the colour theme
 * @param   themeName - the new theme
 *
 * @details Called when the user changes the ColourTheme used.
 */
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
/**
 * @brief   TerminalTab::setKeyboardTheme - switch the keyboard theme
 * @param   themeName - the new theme
 *
 * @details Called when the user changes the KeyboardTheme used.
 */
void TerminalTab::setKeyboardTheme(QString themeName)
{
    //keyboardTheme = themeName;

    // Set keyboard theme by name; pass obtained theme to setKeyboard()

}

/**
 * @brief   TerminalTab::rulerChanged - toggle the ruler
 * @param   on - true to show, false to hide
 *
 * @details Called when the ruler is switched on or off in active settings.
 */
void TerminalTab::rulerChanged(bool on)
{
    // Switch ruler on or off apporpriately
    if (sessionConnected)
    {
        primaryScreen->rulerMode(on);
        alternateScreen->rulerMode(on);
    }
}

/**
 * @brief   TerminalTab::rulerStyle - change the ruler style
 * @param   rulerStyle - the style of ruler
 *
 * @details Called when the ruler style is changed in active settings.
 */
void TerminalTab::rulerStyle(int rulerStyle)
{
    // Change ruler style to match settings
    if (sessionConnected)
    {
        primaryScreen->setRulerStyle(rulerStyle);
        alternateScreen->setRulerStyle(rulerStyle);
    }
}

/**
 * @brief   TerminalTab::openConnection - open a connection
 * @param   address - target address, may contain an LU name
 *
 * @details Called when the user opens a connection with a address of the form luname@targetaddress:port.
 *          This is parsed and then passed to connectSession.
 */
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

/**
 * @brief   TerminalTab::openConnection - open a session
 * @param   s - the session to be opened
 *
 * @details Called when a session is opened. The parameter points to the session already identified in the
 *          config file.
 */
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
    activeSettings.setRulerState(s.value("Ruler").toBool());
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

/**
 * @brief   TerminalTab::connectSession - connect to a host
 * @param   host   - the host to connect to
 * @param   port   - the port to use
 * @param   luName - LU name, may be blank
 *
 * @details Connect to a host. Build the ProcessDataStream, the SocketConnection, the primary and alternate
 *          screens, and connect the Keyboard to them. Set the fonts and colours of the screens.
 *
 *          Start timers to blink the cursor and any blinking characters on screen.
 */
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
    connect(socket, &SocketConnection::connectionEnded, this, &TerminalTab::closeConnection);

    // Primary screen settings
    connect(&activeSettings, &ActiveSettings::cursorInheritChanged, primaryScreen, &DisplayScreen::setCursorColour);
    connect(datastream, &ProcessDataStream::cursorMoved, primaryScreen, &DisplayScreen::showStatusCursorPosition);

    connect(&kbd, &Keyboard::setLock, primaryScreen, &DisplayScreen::setStatusXSystem);
    connect(&kbd, &Keyboard::setInsert, primaryScreen, &DisplayScreen::setStatusInsert);

    primaryScreen->setFont(activeSettings.getFont());
    primaryScreen->rulerMode(activeSettings.getRulerState());
    primaryScreen->setRulerStyle(activeSettings.getRulerStyle());

    // Alternate screen settings
    connect(&activeSettings, &ActiveSettings::cursorInheritChanged, alternateScreen, &DisplayScreen::setCursorColour);
    connect(datastream, &ProcessDataStream::cursorMoved, alternateScreen, &DisplayScreen::showStatusCursorPosition);

    connect(&kbd, &Keyboard::setLock, alternateScreen, &DisplayScreen::setStatusXSystem);
    connect(&kbd, &Keyboard::setInsert, alternateScreen, &DisplayScreen::setStatusInsert);

    alternateScreen->setFont(activeSettings.getFont());
    alternateScreen->rulerMode(activeSettings.getRulerState());
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

    socket->connectMainframe(host, port, luName, datastream);

    startTimers();

    sessionConnected = true;

    // Menu "Connect" entry enable
    emit connectionEstablished();

    kbd.setConnected(true);

    view->setInteractive(true);

    fit();
}

/**
 * @brief   TerminalTab::closeConnection - terminate a connection
 *
 * @details Close the connection, disconnect the Keyboard and delete the primary and alternate screens,
 *          the ProcessDataStream and the SocketConnection. Switch to the 'Not Connected' screen and
 *          stop the blinking timers.
 */
void TerminalTab::closeConnection()
{
    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
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

    socket->disconnectMainframe();

    stopTimers();

    view->setScene(notConnectedScene);

    delete primaryScreen;
    delete alternateScreen;

    delete primary;
    delete alternate;

    delete datastream;
    delete socket;

    sessionConnected = false;

    // Menu "Connect" entry disable
    emit disconnected();

    kbd.setConnected(false);

    view->setInteractive(false);
}

/**
 * @brief   TerminalTab::closeEvent - close the window
 * @param   closeEvent - the close event
 *
 * @details Close the window when the user clicks the 'X' on the top right.
 */
void TerminalTab::closeEvent(QCloseEvent *closeEvent)
{
    if (sessionConnected)
    {
        closeConnection();
    }
    closeEvent->accept();
}

/**
 * @brief   TerminalTab::setBlink - turn cursor blinking on or off
 * @param   blink - true to blink, false to stay static
 *
 * @details Called when the cursor blink preference changes. The cursor is shown if the display
 *          is connected to the host to ensure the cursor doesn't stay hidden if it happened to be in the
 *          'off' state of a blink.
 */
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

/**
 * @brief   TerminalTab::setBlinkSpeed - change the blink speed of the cursor
 * @param   speed - the speed
 *
 * @details The speed is 1 - 4, with 4 being the fastest blink.
 */
void TerminalTab::setBlinkSpeed(int speed)
{
    blinkSpeed = speed;
    cursorBlinker->stop();
    if (blinkSpeed > 0 && blink)
    {
        cursorBlinker->start((5 - blinkSpeed) * 250);
    }
}

/**
 * @brief   TerminalTab::changeCodePage -  change the code page
 *
 * @details Called when the CodePage preference is changed.
 */
void TerminalTab::changeCodePage()
{
    if (sessionConnected)
        current->setCodePage();
}

/**
 * @brief   TerminalTab::stopTimers - stop the blinking timers
 *
 * @details The cursor blink and the character blink timers are stopped.
 */
void TerminalTab::stopTimers()
{
    blinker->stop();
    cursorBlinker->stop();

    disconnect(blinker, &QTimer::timeout, current, &DisplayScreen::blink);
    disconnect(cursorBlinker, &QTimer::timeout, current, &DisplayScreen::cursorBlink);
}

/**
 * @brief   TerminalTab::startTimers - start the blinking timers
 *
 * @details Start the cursor and character blinking timers.
 */
void TerminalTab::startTimers()
{
    connect(blinker, &QTimer::timeout, current, &DisplayScreen::blink);
    connect(cursorBlinker, &QTimer::timeout, current, &DisplayScreen::cursorBlink);

    blinker->start(1000);
    setBlinkSpeed(blinkSpeed);
}

/**
 * @brief   TerminalTab::blinkText - blink any text
 *
 * @details Slot signalled by the character blink timer when it reaches zero. Each iteration
 *          switches from on to off and back.
 */
void TerminalTab::blinkText()
{
    if (sessionConnected)
    {
        current->blink();
    }
}

/**
 * @brief   TerminalTab::blinkCursor - blink the cursor
 *
 * @details Slot signalled by the cursor blink timer when it reaches zero. Each iteration
 *          switches from on to off and back.
 */
void TerminalTab::blinkCursor()
{
    if (sessionConnected)
    {
        current->cursorBlink();
    }
}

/**
 * @brief   TerminalTab::setAlternateScreen - switch screens
 * @param   alt - true for alternate, false for primary
 * @return  the screen switched to
 *
 * @details Change the currently displayed screen in the view, and ensure it fits into the window,
 *          according to user preference. The switched-to screen is returned.
 */
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

    fit();

    return current;
}

/**
 * @brief   TerminalTab::fit - fit the window content according to user preference
 *
 * @details Fit the terminal display into the window, either fixed at 4:3 ratio or stretched to fill
 *          the window. If the 'Not Connected' display is shown, it's always stretched to fill the window.
 */
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
