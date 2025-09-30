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

#include "Terminal.h"

/**
 * @brief   Terminal::Terminal - the terminal in the Qt window
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
Terminal::Terminal(QVBoxLayout *layout, ActiveSettings &activeSettings, CodePage &cp, Keyboard &kb, const Colours &cs) :
    kbd(kb), cp(cp), palette(cs), activeSettings(activeSettings)
{
    sessionConnected = false;
    stretchScreen = Qt::IgnoreAspectRatio;

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
    connect(&activeSettings, &ActiveSettings::rulerStyleChanged, this, &Terminal::rulerStyle);
    connect(&activeSettings, &ActiveSettings::rulerChanged, this, &Terminal::rulerChanged);
    connect(&activeSettings, &ActiveSettings::cursorBlinkChanged, this, &Terminal::setBlink);
    connect(&activeSettings, &ActiveSettings::cursorBlinkSpeedChanged, this, &Terminal::setBlinkSpeed);

    connect(&activeSettings, &ActiveSettings::fontChanged, this, &Terminal::setFont);
    connect(&activeSettings, &ActiveSettings::codePageChanged, this, &Terminal::changeCodePage);
    connect(&activeSettings, &ActiveSettings::stretchScreenChanged, this, &Terminal::setScreenStretch);

//    connect(&activeSettings, &ActiveSettings::colourThemeChanged, this, &Terminal::setColourTheme);

    // Build "Not Connected" display
    notConnectedScene = new QGraphicsScene();

    QGraphicsRectItem *mRect = new QGraphicsRectItem(0, 0, 640, 480);
    mRect->setBrush(QColor(Qt::black));
    mRect->setPen(QColor(Qt::black));

    notConnectedScene->addItem(mRect);

    QGraphicsSimpleTextItem *ncMessage = new QGraphicsSimpleTextItem("Not Connected", mRect);
    ncReason = new QGraphicsSimpleTextItem("", mRect);

    ncMessage->setBrush(QColor(Qt::white));
    ncReason->setBrush(QColor(Qt::red));

    ncMessage->setPen(QColor(Qt::white));
    ncReason->setPen(QColor(Qt::red));

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

    blink      = activeSettings.getCursorBlink();
    blinkSpeed = activeSettings.getCursorBlinkSpeed();
}

/**
 * @brief   Terminal::~Terminal - destructor
 *
 * @details Delete the objects obtained via new.
 */
Terminal::~Terminal()
{
    delete notConnectedScene;
    delete view;
}

/**
 * @brief   Terminal::setFont - change the font on the primary and alternate screens
 * @param   font - the chosen font
 *
 * @details Called when the user changes the font.
 */
void Terminal::setFont(QFont font)
{
    if (sessionConnected)
    {
        primaryScreen->setFont(font);
        alternateScreen->setFont(font);
    }
}

/**
 * @brief   Terminal::setCurrentFont - temporarily change the font on the current screen
 * @param   f - the chosen font
 *
 * @details Called to dynamically change the font on the currently displayed screen. Used during the
 *          Preferences dialog display to show the different fonts as they are selected by the user.
 *
 * @note    This processing is currently disabled.
 */
void Terminal::setCurrentFont(QFont f)
{
    if (sessionConnected)
    {
        current->setFont(f);
    }
}

/**
 * @brief   Terminal::setScreenStretch - toggle the screen stretch and fit the content
 * @param   stretch - true to stretch to fit the window, false to maintain 4:3 ratio
 *
 * @details Called when the user has switched from stretching the screen to fit the window or to
 *          maintain a 4:3 ratio.
 */
void Terminal::setScreenStretch(bool stretch)
{
    stretchScreen = stretch ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio;
    fit();
}

/**
 * @brief   Terminal::setColourTheme - switch the colour theme
 * @param   themeName - the new theme
 *
 * @details Called when the user changes the ColourTheme used.
 */
void Terminal::setColourTheme(const Colours &colours)
{
    palette = colours;

    //FIXME: Might not be needed because of passing palette by reference to DisplayScreen

    if (sessionConnected)
    {
        current->resetColours();
    }
// This from Copilot, is all that is needed:

//void DisplayScreen::resetColours()
//{
//    update(); // marks the whole DisplayScreen (and children) dirty
//}

    // Set colour theme by name; pass obtained theme to setColours()
    if (sessionConnected)
    {
        primary->update();
        alternate->update();
    }
}

/**
 * @brief   Terminal::rulerChanged - toggle the ruler
 * @param   on - true to show, false to hide
 *
 * @details Called when the ruler is switched on or off in active settings.
 */
void Terminal::rulerChanged(bool on)
{
    // Switch ruler on or off apporpriately
    if (sessionConnected)
    {
        primaryScreen->rulerMode(on);
        alternateScreen->rulerMode(on);
    }
}

/**
 * @brief   Terminal::rulerStyle - change the ruler style
 * @param   rulerStyle - the style of ruler
 *
 * @details Called when the ruler style is changed in active settings.
 */
void Terminal::rulerStyle(Q3270::RulerStyle rulerStyle)
{
    // Change ruler style to match settings
    if (sessionConnected)
    {
        primaryScreen->setRulerStyle(rulerStyle);
        alternateScreen->setRulerStyle(rulerStyle);
    }
}

/**
 * @brief   Terminal::connectSession - connect to a host
 * @param   host   - the host to connect to
 * @param   port   - the port to use
 * @param   luName - LU name, may be blank
 *
 * @details Connect to a host. Build the ProcessDataStream, the SocketConnection, the primary and alternate
 *          screens, and connect the Keyboard to them. Set the fonts and colours of the screens.
 *
 *          Start timers to blink the cursor and any blinking characters on screen.
 */
void Terminal::connectSession()
{
    setWindowTitle(QString('[').append(activeSettings.getSessionName()).append(']'));

    // Set up primary and alternate scenes
    primary = new QGraphicsScene();
    alternate = new QGraphicsScene();

    primaryScreen = new DisplayScreen(80, 24, cp, &palette, primary);
    alternateScreen = new DisplayScreen(activeSettings.getTerminalX(), activeSettings.getTerminalY(), cp, &palette, alternate);

    current = primaryScreen;
    view->setScene(primary);

    datastream = new ProcessDataStream(this);
    socket = new SocketConnection(activeSettings.getTerminalModel());

    socket->setSecure(activeSettings.getSecureMode());
    socket->setVerify(activeSettings.getVerifyCerts());

    connect(datastream, &ProcessDataStream::bufferReady, socket, &SocketConnection::sendResponse);

    connect(primaryScreen, &DisplayScreen::bufferReady, socket, &SocketConnection::sendResponse);
    connect(alternateScreen, &DisplayScreen::bufferReady, socket, &SocketConnection::sendResponse);

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);

    connect(socket, &SocketConnection::encryptedConnection, primaryScreen, &DisplayScreen::setEncrypted);
    connect(socket, &SocketConnection::encryptedConnection, alternateScreen, &DisplayScreen::setEncrypted);

    connect(socket, &SocketConnection::connectionEnded, this, &Terminal::closeConnection);

    primaryScreen->setFont(activeSettings.getFont());
    primaryScreen->rulerMode(activeSettings.getRulerState());
    primaryScreen->setRulerStyle(activeSettings.getRulerStyle());

    alternateScreen->setFont(activeSettings.getFont());
    alternateScreen->rulerMode(activeSettings.getRulerState());
    alternateScreen->setRulerStyle(activeSettings.getRulerStyle());

    // Status bar updates
    connect(datastream, &ProcessDataStream::processingComplete, this, &Terminal::clearTWait);
    connect(datastream, &ProcessDataStream::unlockKeyboard, this, &Terminal::resetStatusXSystem);

    connect(&kbd, &Keyboard::key_Reset, this, &Terminal::resetStatusXSystem);
    connect(&kbd, &Keyboard::setEnterInhibit, this, &Terminal::setTWait);
    connect(&kbd, &Keyboard::setInsert, this, &Terminal::setStatusInsert);


//    connect(&activeSettings, &ActiveSettings::cursorInheritChanged, &screen, &DisplayScreen::setCursorColour);
//    connect(datastream, &ProcessDataStream::cursorMoved, &screen, &DisplayScreen::showStatusCursorPosition);

//    connectKeyboard(*current);

    // Keyboard inputs
    connect(&kbd, &Keyboard::key_Copy, this, &Terminal::copyText);

    socket->connectMainframe(activeSettings.getHostName(), activeSettings.getHostPort(), activeSettings.getHostLU(), datastream);

    sessionConnected = true;

    // Menu "Connect" entry enable
    emit connectionEstablished();

    kbd.setConnected(true);

    view->setInteractive(true);

    fit();
}

/**
 * @brief   Terminal::closeConnection - terminate a connection
 *
 * @details Close the connection, disconnect the Keyboard and delete the primary and alternate screens,
 *          the ProcessDataStream and the SocketConnection. Switch to the 'Not Connected' screen and
 *          stop the blinking timers.
 */
void Terminal::closeConnection(QString message)
{
    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);

    disconnect(socket, &SocketConnection::connectionEnded, this, &Terminal::closeConnection);

    disconnect(datastream, &ProcessDataStream::bufferReady, socket, &SocketConnection::sendResponse);

    disconnect(primaryScreen, &DisplayScreen::bufferReady, socket, &SocketConnection::sendResponse);
    disconnect(alternateScreen, &DisplayScreen::bufferReady, socket, &SocketConnection::sendResponse);

    disconnectKeyboard(*primaryScreen);
    disconnectKeyboard(*alternateScreen);

    // Status bar updates
    disconnect(datastream, &ProcessDataStream::unlockKeyboard, this, &Terminal::clearTWait);
    disconnect(&kbd, &Keyboard::key_Reset, this, &Terminal::resetStatusXSystem);
    disconnect(&kbd, &Keyboard::key_Copy, this, &Terminal::copyText);

    socket->disconnectMainframe();

    stopTimers();

    if (!message.isEmpty())
    {
        QFont font("mono", 12);
        // Centre not connected reason based on font size. 640x480 halved, less the size of the font
        QFontMetrics fm(font);
        ncReason->setText(message);
        ncReason->setFont(font);
        int xPos = 320 - fm.horizontalAdvance(ncReason->text()) / 2;
        int yPos = 320 - fm.height() / 2;
        ncReason->setPos(xPos, yPos);
    }
    else
    {
        ncReason->setText("");
    }

    view->setScene(notConnectedScene);

    disconnect(socket, &SocketConnection::encryptedConnection, primaryScreen, &DisplayScreen::setEncrypted);
    disconnect(socket, &SocketConnection::encryptedConnection, alternateScreen, &DisplayScreen::setEncrypted);

    delete primaryScreen;
    delete alternateScreen;

    delete primary;
    delete alternate;

    delete datastream;
    socket->deleteLater();

    sessionConnected = false;

    // Menu "Connect" entry disable
    emit disconnected();

    kbd.setConnected(false);

    view->setInteractive(false);
}

/**
 * @brief   Terminal::connectKeyboard - Connect keyboard
 * @param   screen - the screen
 *
 * @details Connect the keyboard to the specified screen. Used to ensure the keyboard is connected to either the
 *          primary or alternate.
 */
void Terminal::connectKeyboard(DisplayScreen &screen)
{
    connect(&kbd, &Keyboard::key_Character, &screen, &DisplayScreen::insertChar);

    connect(&kbd, &Keyboard::key_Home, &screen, &DisplayScreen::home);
    connect(&kbd, &Keyboard::key_Backspace, &screen, &DisplayScreen::backspace);
    connect(&kbd, &Keyboard::key_Delete, &screen, &DisplayScreen::deleteChar);
    connect(&kbd, &Keyboard::key_EraseEOF, &screen, &DisplayScreen::eraseEOF);
    connect(&kbd, &Keyboard::key_Newline, &screen, &DisplayScreen::newline);
    connect(&kbd, &Keyboard::key_Tab, &screen, &DisplayScreen::tab);
    connect(&kbd, &Keyboard::key_End, &screen, &DisplayScreen::endline);
    connect(&kbd, &Keyboard::key_Backtab, &screen, &DisplayScreen::backtab);
    connect(&kbd, &Keyboard::key_moveCursor, &screen, &DisplayScreen::moveCursor);
    connect(&kbd, &Keyboard::key_toggleRuler, &screen, &DisplayScreen::toggleRuler);
    connect(&kbd, &Keyboard::key_showInfo, &screen, &DisplayScreen::dumpInfo);
    connect(&kbd, &Keyboard::key_showFields, &screen, &DisplayScreen::dumpFields);
    connect(&kbd, &Keyboard::key_Attn, &screen, &DisplayScreen::interruptProcess);
    connect(&kbd, &Keyboard::key_AID, &screen, &DisplayScreen::processAID);
}

/**
 * @brief   Terminal::disconnectKeyboard - disconnect keyboard
 * @param   screen - the screen
 *
 * @details Disconnect the keyboard from the specified screen.
 */
void Terminal::disconnectKeyboard(DisplayScreen &screen)
{
    disconnect(&kbd, &Keyboard::key_Character, &screen, &DisplayScreen::insertChar);

    disconnect(&kbd, &Keyboard::key_Home, &screen, &DisplayScreen::home);
    disconnect(&kbd, &Keyboard::key_Backspace, &screen, &DisplayScreen::backspace);
    disconnect(&kbd, &Keyboard::key_Delete, &screen, &DisplayScreen::deleteChar);
    disconnect(&kbd, &Keyboard::key_EraseEOF, &screen, &DisplayScreen::eraseEOF);
    disconnect(&kbd, &Keyboard::key_Newline, &screen, &DisplayScreen::newline);
    disconnect(&kbd, &Keyboard::key_Tab, &screen, &DisplayScreen::tab);
    disconnect(&kbd, &Keyboard::key_End, &screen, &DisplayScreen::endline);
    disconnect(&kbd, &Keyboard::key_Backtab, &screen, &DisplayScreen::backtab);
    disconnect(&kbd, &Keyboard::key_moveCursor, &screen, &DisplayScreen::moveCursor);
    disconnect(&kbd, &Keyboard::key_toggleRuler, &screen, &DisplayScreen::toggleRuler);
    disconnect(&kbd, &Keyboard::key_showInfo, &screen, &DisplayScreen::dumpInfo);
    disconnect(&kbd, &Keyboard::key_showFields, &screen, &DisplayScreen::dumpFields);
    disconnect(&kbd, &Keyboard::key_Attn, &screen, &DisplayScreen::interruptProcess);
    disconnect(&kbd, &Keyboard::key_AID, &screen, &DisplayScreen::processAID);
}


/**
 * @brief   Terminal::closeEvent - close the window
 * @param   closeEvent - the close event
 *
 * @details Close the window when the user clicks the 'X' on the top right.
 */
void Terminal::closeEvent(QCloseEvent *closeEvent)
{
    if (sessionConnected)
    {
        closeConnection();
    }
    closeEvent->accept();
}

/**
 * @brief   Terminal::setBlink - turn cursor blinking on or off
 * @param   blink - true to blink, false to stay static
 *
 * @details Called when the cursor blink preference changes. The cursor is shown if the display
 *          is connected to the host to ensure the cursor doesn't stay hidden if it happened to be in the
 *          'off' state of a blink.
 */
void Terminal::setBlink(bool blink)
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
 * @brief   Terminal::setBlinkSpeed - change the blink speed of the cursor
 * @param   speed - the speed
 *
 * @details The speed is 1 - 4, with 4 being the fastest blink.
 */
void Terminal::setBlinkSpeed(int speed)
{
    blinkSpeed = speed;
    cursorBlinker->stop();
    if (blinkSpeed > 0 && blink)
    {
        cursorBlinker->start((5 - blinkSpeed) * 250);
    }
}

/**
 * @brief   Terminal::changeCodePage -  change the code page
 *
 * @details Called when the CodePage preference is changed.
 */
void Terminal::changeCodePage(QString codepage)
{
    cp.setCodePage(codepage);

    if (sessionConnected)
        current->setCodePage();
}

/**
 * @brief   Terminal::stopTimers - stop the blinking timers
 *
 * @details The cursor blink and the character blink timers are stopped.
 */
void Terminal::stopTimers()
{
    blinker->stop();
    cursorBlinker->stop();

    disconnect(blinker, &QTimer::timeout, this, &Terminal::blinkText);
    disconnect(cursorBlinker, &QTimer::timeout, this, &Terminal::blinkCursor);

    current->showCursor();
}

/**
 * @brief   Terminal::startTimers - start the blinking timers
 *
 * @details Start the cursor and character blinking timers.
 */
void Terminal::startTimers()
{
    connect(blinker, &QTimer::timeout, this, &Terminal::blinkText);
    connect(cursorBlinker, &QTimer::timeout, this, &Terminal::blinkCursor);

    blinker->start(1000);
    setBlinkSpeed(blinkSpeed);
}

/**
 * @brief   Terminal::blinkText - blink any text
 *
 * @details Slot signalled by the character blink timer when it reaches zero. Each iteration
 *          switches from on to off and back.
 */
void Terminal::blinkText()
{
    if (sessionConnected)
    {
        current->blink();
        if (!shortCharacterBlink)
        {
            blinker->setInterval(250);
        }
        else
        {
            blinker->setInterval(1000);
        }

        shortCharacterBlink = !shortCharacterBlink;
    }
}

/**
 * @brief   Terminal::blinkCursor - blink the cursor
 *
 * @details Slot signalled by the cursor blink timer when it reaches zero. Each iteration
 *          switches from on to off and back.
 */
void Terminal::blinkCursor()
{
    if (sessionConnected)
    {
        current->cursorBlink();
        if (!shortCursorBlink)
        {
            cursorBlinker->setInterval(250);
        }
        else
        {
            cursorBlinker->setInterval((5 - blinkSpeed) * 250);
        }

        shortCursorBlink = !shortCursorBlink;
    }
}

/**
 * @brief   Terminal::setAlternateScreen - switch screens
 * @param   alt - true for alternate, false for primary
 * @return  the screen switched to
 *
 * @details Change the currently displayed screen in the view, connect the keyboard to the right screen, and ensure it
 *          fits into the window, according to user preference. The switched-to screen is returned.
 */
DisplayScreen *Terminal::setAlternateScreen(bool alt)
{
    stopTimers();

    // Initially the cursor and characters are shown
    shortCursorBlink = false;
    shortCharacterBlink = false;

    if (!alt)
    {
        disconnectKeyboard(*alternateScreen);
        connectKeyboard(*primaryScreen);
        view->setScene(primary);
        current = primaryScreen;
    }
    else
    {
        disconnectKeyboard(*primaryScreen);
        connectKeyboard(*alternateScreen);
        view->setScene(alternate);
        current = alternateScreen;
    }

    startTimers();

    fit();

    return current;
}

void Terminal::setTWait()
{
    if (!sessionConnected)
        return;

    xClock = true;
    xSystem = true;

    updateLockState();

    qDebug() << "TWAIT set";
}

void Terminal::clearTWait()
{
    if (!sessionConnected)
        return;

    xClock = false;

    updateLockState();

    qDebug() << "TWAIT cleared";
}

void Terminal::setStatusInsert(const bool insert)
{
    if (!sessionConnected)
        return;

    current->setStatusInsert(insert ? Q3270::InsertMode : Q3270::OvertypeMode);
}

void Terminal::resetStatusXSystem()
{
    if (!sessionConnected)
        return;

    if (xClock)
        return;

    xSystem = false;

    updateLockState();

    qDebug() << "System Lock cleared";
}

void Terminal::updateLockState()
{
    if (xClock) {
        current->setStatusLock(Q3270::TerminalWait);
        kbd.setLocked(true);
    }
    else if (xSystem) {
        current->setStatusLock(Q3270::SystemLock);
        kbd.setLocked(true);
    }
    else {
        current->setStatusLock(Q3270::Unlocked);
        kbd.setLocked(false);
    }
}


/**
 * @brief   Terminal::fit - fit the window content according to user preference
 *
 * @details Fit the terminal display into the window, either fixed at 4:3 ratio or stretched to fill
 *          the window. If the 'Not Connected' display is shown, it's always stretched to fill the window.
 */
void Terminal::fit()
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
