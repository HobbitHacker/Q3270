/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "Terminal.h"
#include <QDateTime>

#include "Display/StatusBar.h"

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
Terminal::Terminal(QGraphicsView *screen, ActiveSettings &activeSettings, CodePage &cp, Keyboard &kb, const Colours &cs)
    : kbd(kb)
    , cp(cp)
    , palette(cs)
    , activeSettings(activeSettings)
    , screen(screen)
{
    QGraphicsScene *screenScene = new QGraphicsScene(this);

    screen->setScene(screenScene);

    sessionConnected = false;
    stretchScreen = Qt::IgnoreAspectRatio;

    screen->setBackgroundBrush(QBrush(Qt::black));
    screen->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    screen->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    screen->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    screen->setInteractive(false);
    screen->setAlignment(Qt::AlignCenter);
    screen->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    // Plug the keyboard in
    screen->installEventFilter(&kbd);

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
    notConnected = new QGraphicsRectItem(0, 0, 640, 480);
    notConnected->setBrush(QColor(Qt::black));
    notConnected->setPen(QColor(Qt::black));


    QGraphicsSimpleTextItem *ncMessage = new QGraphicsSimpleTextItem("Not Connected", notConnected);
    ncReason = new QGraphicsSimpleTextItem("", notConnected);

    ncMessage->setBrush(QColor(Qt::white));
    ncReason->setBrush(QColor(Qt::red));

    ncMessage->setPen(QColor(Qt::white));
    ncReason->setPen(QColor(Qt::red));

    QFont font("Courier", 12);
    ncMessage->setFont(font);

    // Centre "Not Connected" based on font size. 640x480 halved, less the size of the font
    QFontMetrics fm(font);
    int xPos = 320 - fm.horizontalAdvance(ncMessage->text()) / 2;
    int yPos = 240 - fm.height() / 2;
    ncMessage->setPos(xPos, yPos);

    // Set "Not Connected" as the initially displayed scene
    screen->scene()->addItem(notConnected);
    screen->show();

    fit();

    statusBar = new StatusBar(80 * CELL_WIDTH, CELL_HEIGHT * .90);

    // Blink timers
    blinker = new QTimer(this);
    cursorBlinker = new QTimer(this);

    blink      = activeSettings.getCursorBlink();
    blinkSpeed = activeSettings.getCursorBlinkSpeed();

    current = new DisplayScreen(80, 24, cp, &palette);
}

/**
 * @brief   Terminal::~Terminal - destructor
 *
 * @details Delete the objects obtained via new.
 */
Terminal::~Terminal()
{
    delete current;
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
        current->setFont(font);
    }
}

/**
 * @brief   Terminal::setCurrentFont - temporarily change the font on the current screen
 * @param   f - the chosen font
 *
 * @details Called to dynamically change the font on the currently displayed screen. Used during the
 *          Preferences dialog display to show the different fonts as they are selected by the user.* 
 */
void Terminal::setCurrentFont(QFont f)
{
    if (sessionConnected)
    {
        current->setFont(f);
        current->update();
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
    if (sessionConnected)
    {
        screen->scene()->update();
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
        current->rulerMode(on);
        // primaryScreen->rulerMode(on);
        // alternateScreen->rulerMode(on);
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
        current->setRulerStyle(rulerStyle);
        // primaryScreen->setRulerStyle(rulerStyle);
        // alternateScreen->setRulerStyle(rulerStyle);
    }
}

/**
 * @brief   Terminal::connectSession - connect to a host
 *
 * @details Connect to a host. Build the ProcessDataStream, the SocketConnection, the display matrix
 *          and connect the Keyboard. Set the fonts and colours of the screens.
 *
 *          Start timers to blink the cursor and any blinking characters on screen.
 */
void Terminal::connectSession()
{
    setWindowTitle(QString('[').append(activeSettings.getSessionName()).append(']'));

    screen->scene()->removeItem(notConnected);
    screen->scene()->addItem(current);
    screen->scene()->addItem(statusBar);

    statusBar->setPos(0, current->boundingRect().height());

    datastream = new ProcessDataStream(this, current);
    socket = new SocketConnection(activeSettings.getTerminalModel());

    socket->setSecure(activeSettings.getSecureMode());
    socket->setVerify(activeSettings.getVerifyCerts());

    connect(datastream, &ProcessDataStream::bufferReady, socket, &SocketConnection::sendResponse);
    connect(datastream, &ProcessDataStream::setAlternateScreen, this, &Terminal::setAlternateScreen);

    connect(current, &DisplayScreen::bufferReady, socket, &SocketConnection::sendResponse);

    connect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);
    connect(socket, &SocketConnection::encryptedConnection, statusBar, &StatusBar::setEncrypted);
    connect(socket, &SocketConnection::connectionEnded, this, &Terminal::closeConnection);

    current->setFont(activeSettings.getFont());
    current->rulerMode(activeSettings.getRulerState());
    current->setRulerStyle(activeSettings.getRulerStyle());

    // Status bar updates
    connect(datastream, &ProcessDataStream::processingComplete, this, &Terminal::clearTWait);
    connect(datastream, &ProcessDataStream::unlockKeyboard, this, &Terminal::resetStatusXSystem);

    connect(&kbd, &Keyboard::key_Reset, this, &Terminal::resetStatusXSystem);
    connect(&kbd, &Keyboard::setEnterInhibit, this, &Terminal::setTWait);
    connect(&kbd, &Keyboard::setInsert, this, &Terminal::setStatusInsert);

    // Keyboard inputs
    connect(&kbd, &Keyboard::key_Copy, this, &Terminal::copyText);

    connectKeyboard();

    socket->connectMainframe(activeSettings.getHostName(), activeSettings.getHostPort(), activeSettings.getHostLU(), datastream);

    startTimers();

    sessionConnected = true;

    // Menu "Connect" entry enable
    emit connectionEstablished();

    kbd.setConnected(true);

    screen->setInteractive(true);

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
    sessionConnected = false;

    disconnect(socket, &SocketConnection::dataStreamComplete, datastream, &ProcessDataStream::processStream);

    disconnect(socket, &SocketConnection::connectionEnded, this, &Terminal::closeConnection);

    disconnect(datastream, &ProcessDataStream::bufferReady, socket, &SocketConnection::sendResponse);
    disconnect(datastream, &ProcessDataStream::setAlternateScreen, this, &Terminal::setAlternateScreen);

    disconnect(current, &DisplayScreen::bufferReady, socket, &SocketConnection::sendResponse);

    disconnectKeyboard();

    // Status bar updates
    disconnect(datastream, &ProcessDataStream::unlockKeyboard, this, &Terminal::resetStatusXSystem);
    disconnect(&kbd, &Keyboard::key_Reset, this, &Terminal::resetStatusXSystem);
    disconnect(&kbd, &Keyboard::key_Copy, this, &Terminal::copyText);
    disconnect(&kbd, &Keyboard::setEnterInhibit, this, &Terminal::setTWait);
    disconnect(&kbd, &Keyboard::setInsert, this, &Terminal::setStatusInsert);

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

    screen->scene()->removeItem(current);
    screen->scene()->removeItem(statusBar);
    screen->scene()->addItem(notConnected);

    fit();

    disconnect(socket, &SocketConnection::encryptedConnection, statusBar, &StatusBar::setEncrypted);

    delete datastream;
    socket->deleteLater();

    // Menu "Connect" entry disable
    emit disconnected();

    kbd.setConnected(false);

    screen->setInteractive(false);
}

/**
 * @brief   Terminal::connectKeyboard - Connect keyboard
 * @param   screen - the screen
 *
 * @details Connect the keyboard to the specified screen. Used to ensure the keyboard is connected to either the
 *          primary or alternate.
 */
void Terminal::connectKeyboard()
{
    connect(&kbd, &Keyboard::key_Character, current, &DisplayScreen::insertChar);

    connect(&kbd, &Keyboard::key_Home, current, &DisplayScreen::home);
    connect(&kbd, &Keyboard::key_Backspace, current, &DisplayScreen::backspace);
    connect(&kbd, &Keyboard::key_Delete, current, &DisplayScreen::deleteChar);
    connect(&kbd, &Keyboard::key_EraseEOF, current, &DisplayScreen::eraseEOF);
    connect(&kbd, &Keyboard::key_Newline, current, &DisplayScreen::newline);
    connect(&kbd, &Keyboard::key_Tab, current, &DisplayScreen::tab);
    connect(&kbd, &Keyboard::key_End, current, &DisplayScreen::endline);
    connect(&kbd, &Keyboard::key_Backtab, current, &DisplayScreen::backtab);
    connect(&kbd, &Keyboard::key_moveCursor, current, &DisplayScreen::moveCursor);
    connect(&kbd, &Keyboard::key_toggleRuler, current, &DisplayScreen::toggleRuler);
    connect(&kbd, &Keyboard::key_showInfo, current, &DisplayScreen::dumpInfo);
    connect(&kbd, &Keyboard::key_showFields, current, &DisplayScreen::dumpFields);
    connect(&kbd, &Keyboard::key_dumpScreen, current, &DisplayScreen::dumpDisplay);
    connect(&kbd, &Keyboard::key_Attn, current, &DisplayScreen::interruptProcess);
    connect(&kbd, &Keyboard::key_AID, current, &DisplayScreen::processAID);
    
    connect(current, &DisplayScreen::cursorMoved, statusBar, &StatusBar::cursorMoved);
}

/**
 * @brief   Terminal::disconnectKeyboard - disconnect keyboard
 * @param   screen - the screen
 *
 * @details Disconnect the keyboard from the specified screen.
 */
void Terminal::disconnectKeyboard()
{
    disconnect(&kbd, &Keyboard::key_Character, current, &DisplayScreen::insertChar);

    disconnect(&kbd, &Keyboard::key_Home, current, &DisplayScreen::home);
    disconnect(&kbd, &Keyboard::key_Backspace, current, &DisplayScreen::backspace);
    disconnect(&kbd, &Keyboard::key_Delete, current, &DisplayScreen::deleteChar);
    disconnect(&kbd, &Keyboard::key_EraseEOF, current, &DisplayScreen::eraseEOF);
    disconnect(&kbd, &Keyboard::key_Newline, current, &DisplayScreen::newline);
    disconnect(&kbd, &Keyboard::key_Tab, current, &DisplayScreen::tab);
    disconnect(&kbd, &Keyboard::key_End, current, &DisplayScreen::endline);
    disconnect(&kbd, &Keyboard::key_Backtab, current, &DisplayScreen::backtab);
    disconnect(&kbd, &Keyboard::key_moveCursor, current, &DisplayScreen::moveCursor);
    disconnect(&kbd, &Keyboard::key_toggleRuler, current, &DisplayScreen::toggleRuler);
    disconnect(&kbd, &Keyboard::key_showInfo, current, &DisplayScreen::dumpInfo);
    disconnect(&kbd, &Keyboard::key_showFields, current, &DisplayScreen::dumpFields);
    disconnect(&kbd, &Keyboard::key_dumpScreen, current, &DisplayScreen::dumpDisplay);
    disconnect(&kbd, &Keyboard::key_Attn, current, &DisplayScreen::interruptProcess);
    disconnect(&kbd, &Keyboard::key_AID, current, &DisplayScreen::processAID);
    
    disconnect(current, &DisplayScreen::cursorMoved, statusBar, &StatusBar::cursorMoved);
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
 * @details Resize the screen matrix, and ensure it fits into the window, according to user preference.
 */
DisplayScreen *Terminal::setAlternateScreen(bool alt)
{
    stopTimers();

    if (!alt)
        current->setSize(80, 24);
    else
        current->setSize(activeSettings.getTerminalX(), activeSettings.getTerminalY());

    statusBar->setPos(0, current->boundingRect().height());
    statusBar->setSize(current->boundingRect().width(), CELL_HEIGHT * 0.90);

    QRectF nr = current->boundingRect().united(statusBar->mapToScene(statusBar->boundingRect()).boundingRect());

    screen->scene()->setSceneRect(nr);

    screen->resetTransform();

    startTimers();

    fit();

    return current;
}

/**
 * @brief   Terminal::setStatusInsert - update the status bar with insert mode
 * @param   insert - Q3270::InsertMode or Q3270::OvertypeMode
 *
 * @details setStatusInsert is called when the user toggles the insert mode with the
 *          insert key.
 */
void Terminal::setStatusInsert(const bool insert)
{
    if (!sessionConnected)
        return;

    statusBar->setStatusInsert(insert ? Q3270::InsertMode : Q3270::OvertypeMode);
}

/**
 * @brief   Terminal::setTWait - Set the TWAIT condition
 *
 * @details setTWait is called when the terminal is waiting for a response from the host.
 *          TWAIT is when the 'X <clock>' is shown.
 */
void Terminal::setTWait()
{
    if (!sessionConnected)
        return;

    xClock = true;
    xSystem = true;

    updateLockState();
}

/**
 * @brief   Terminal::clearTWait - Clear the TWAIT condition
 *
 * @details clearTWait is called when the host has responded. It does not clear X System.
 */
void Terminal::clearTWait()
{
    if (!sessionConnected)
        return;

    xClock = false;

    updateLockState();

    current->update();
}

/**
 * @brief   Terminal::resetStatusXSystem - Clear X System condition if possible
 *
 * @details resetStatusXSystem is called when the user presses the Reset key or the
 *          host has responded to a previous data stream. If TWAIT is set, this
 *          is ignored.
 */
void Terminal::resetStatusXSystem()
{
    if (!sessionConnected)
        return;

    if (xClock)
        return;

    xSystem = false;

    updateLockState();
}

/**
 * @brief   Terminal::updateLockState - Update the X <clock>, X System status
 *
 * @details updateLockState refreshes the status bar for X Clock or X System, or unlocked.
 */
void Terminal::updateLockState()
{
    if (xClock) {
        statusBar->setStatusLock(Q3270::TerminalWait);
        kbd.setLocked(true);
    }
    else if (xSystem) {
        statusBar->setStatusLock(Q3270::SystemLock);
        kbd.setLocked(true);
    }
    else {
        statusBar->setStatusLock(Q3270::Unlocked);
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
        screen->fitInView(screen->scene()->itemsBoundingRect(), stretchScreen);
    }
    else
    {
        screen->fitInView(notConnected, Qt::IgnoreAspectRatio);
    }
}
