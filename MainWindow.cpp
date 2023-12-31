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

#include "ui_MainWindow.h"
#include "MainWindow.h"
#include "ui_About.h"

/**
 * @brief   MainWindow::MainWindow - the main application
 * @param   s - a name
 *
 * @details Initialise the application, and open any session name passed to it. The session
 *          is a struct that contains an existing MainWindow or null if this is the first one,
 *          and any session name to be opened.
 */
MainWindow::MainWindow(MainWindow::Session s) : QMainWindow(nullptr),
                                                ui(new(Ui::MainWindow))
{

    QCoreApplication::setOrganizationDomain("styles.homeip.net");
    QCoreApplication::setApplicationName("Q3270");
    QCoreApplication::setOrganizationName("andyWare");

    ui->setupUi(this);

    // Read global settings
    QSettings savedSettings;

    // Most-recently used; default to 10
    maxMruCount = savedSettings.value("MRUMax", 10).toInt();

    // Most-recently used list; might be addresses or sessions
    int mruCount = savedSettings.beginReadArray("RecentlyUsed");

    for(int i = 0; i < mruCount; i++)
    {
        savedSettings.setArrayIndex(i);

        // Pick up each Recently Used entry
        QString thisEntry = savedSettings.value("Entry").toString();

        // Store the entry
        mruList.append(thisEntry);

        // Insert an entry MRU menu list with a number
        // If it starts with Session, use as is, otherwise, it's an address, so add 'Host' to the menu entry
        if (thisEntry.startsWith("Session"))
        {
            thisEntry = QString::number(i + 1) + ". " + thisEntry;
        }
        else
        {
            thisEntry = QString::number(i + 1) + ". Host " + thisEntry;
        }
        ui->menuRecentSessions->addAction(thisEntry, this, &MainWindow::mruConnect);
    }

    savedSettings.endArray();

    sessionGroup = new QActionGroup(this);

    // Preferences dialog
    settings = new PreferencesDialog(colourTheme, keyboardTheme, activeSettings);

    // FIXME: is this bad?
    // Populate code page list
    settings->populateCodePages(codePage.getCodePageList());

    // Session Management dialog
    sm = new SessionManagement(activeSettings);

    // Update MRU entries if the user opens a session
    connect(sm, &SessionManagement::sessionOpened, this, &MainWindow::updateMRUlist);

    // Change Connect menu entry when connected, or when user fills in hostname field in Settings
    connect(settings, &PreferencesDialog::connectValid, this, &MainWindow::enableConnectMenu);

    // Set defaults for Connect options
    ui->actionDisconnect->setDisabled(true);
    
    terminal = new Terminal(ui->terminalLayout, activeSettings, codePage, keyboard, colourTheme, s.session);

    keyboard.setTheme(keyboardTheme, "Factory");

    // Used for dynamically showing font changes when using the font selection dialog
    connect(settings, &PreferencesDialog::tempFontChange, terminal, &Terminal::setCurrentFont);

    // Enable/Disable menu entries if connected/disconnected
    connect(terminal, &Terminal::disconnected, this, &MainWindow::disableDisconnectMenu);
    connect(terminal, &Terminal::disconnected, settings, &PreferencesDialog::disconnected);
    
    connect(terminal, &Terminal::connectionEstablished, this, &MainWindow::enableDisconnectMenu);
    connect(terminal, &Terminal::connectionEstablished, settings, &PreferencesDialog::connected);

    // If a session name was passed to the MainWindow, restore the window size/position
    // and open it
    if (!s.session.isEmpty())
    {
        //TODO: Decide if Window Geometry is just going to mean that a session always has the same pos etc on screen
        //TODO: Use QWidget.resize and QWidget.move instead. See QSettings doc
        restoreGeometry(savedSettings.value(s.session + "/WindowGeometry").toByteArray());
        sm->openSession(terminal, QUrl::fromPercentEncoding(s.session.toLatin1()));

        // Enable Save Session menu item
        ui->actionSave_Session->setEnabled(true);

        // Disable/Enable Reconnect etc menu entries
        ui->actionDisconnect->setEnabled(true);
    }
    else
    {
        // This is not a named session, so disable Save Session menu item
        ui->actionSave_Session->setDisabled(true);
    }

    // If there's none but this window, it must be initial start
    if (s.mw == nullptr)
    {
        int autoStart = savedSettings.beginReadArray("AutoStart");
        for(int i = 0; i < autoStart; i++)
        {
            savedSettings.setArrayIndex(i);

            // If there's more than one window to be opened, start a new one, but for the first one,
            // open the named session.
            if (i > 0)
            {
                MainWindow *newWindow = new MainWindow({ this, savedSettings.value("Session").toString() } );
                newWindow->show();
            }
            else
            {
                //TODO check if we actually need the fromPercentEncoding
                sm->openSession(terminal, QUrl::fromPercentEncoding(savedSettings.value("Session").toString().toLatin1()));

                // Enable Save Session menu entry as it's a named session
                ui->actionSave_Session->setEnabled(true);

                // Disable/Enable Reconnect etc menu entries
                ui->actionDisconnect->setEnabled(true);
            }
        }

        savedSettings.endArray();
    }

    qDebug() << ui->centralwidget->size();

}

/**
 * @brief   MainWindow::menuNew - start a new window
 *
 * @details Start a new window without a session
 */
void MainWindow::menuNew()
{
    MainWindow *newWindow = new MainWindow( { this, "" });
    newWindow->show();
}

/**
 * @brief   MainWindow::menuDuplicate - start a duplicate window
 *
 * @details Start a new window with the same session name as the existing one
 */
void MainWindow::menuDuplicate()
{
    MainWindow *newWindow = new MainWindow({ this, terminal->getSessionName() });
    newWindow->show();
}

/**
 * @brief   MainWindow::menuSaveSession - save the current session
 *
 * @details Save the current session settings to the config file.
 */
void MainWindow::menuSaveSession()
{
    sm->saveSettings();
}

/**
 * @brief   MainWindow::menuSaveSessionAs - save current session as a new name
 *
 * @details Save the current session settings to the config file with a new session name.
 */
void MainWindow::menuSaveSessionAs()
{
    // Save Session dialog, setting the Save Session menu entry dis/enabled
    ui->actionSave_Session->setEnabled(sm->saveSessionAs());
}

/**
 * @brief   MainWindow::menuOpenSession - open a session
 *
 * @details Open a session
 */
void MainWindow::menuOpenSession()
{
    // Open Session dialog
    ui->actionSave_Session->setEnabled(sm->openSession(terminal));
}

/**
 * @brief   MainWindow::menuManageSessions - Open the manage session dialog
 *
 * @details Open the manage sessions dialog to allow the user to add/delete sessions.
 */
void MainWindow::menuManageSessions()
{
    // Manage Sessions Dialog
    sm->manageSessions();
}

/**
 * @brief   MainWindow::mruConnect - open a previously used setting
 *
 * @details Invoked when the user has selected a previously used setting from the most recently
 *          used list. This may be a session name, or just a simple host name and port. If it
 *          is not a session, the Save session option is greyed out.
 */
void MainWindow::mruConnect()
{
    // Find the sending object - the line in the 'Recent' menu
    QAction *sender = (QAction *)QObject::sender();

    // Split the menu entry into parts: 1. Host 192.168.0.1:23 or 1. Session Fred
    QStringList parts = sender->text().split(" ");

    // If this is a Host address, use that to connect to
    if (!parts[1].compare("Host"))
    {
        // Pick up address and connect
        terminal->openConnection(parts[2]);

        // Set Connect menu entries
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);

        // Not a named session, so can't save it as one (use Save Session As.. instead)
        ui->actionSave_Session->setDisabled(true);

        // Update recently used list
        updateMRUlist("Host " + parts[2]);

        // Update the address on the Host form
        activeSettings.setHostAddress(parts[2]);
    }
    else
    {
        // Will contain either session name or host address
        QString address;

        // It's a session name, so concatenate the remaining parts of the menu entry
        // as the session name may have embedded spaces
        for(int i = 2; i < parts.count(); i++)
        {
            address = address.append(parts[i] + " ");
        }

        // Remove last trailing space added above
        address.chop(1);

        // Open the session, which also emits an update to the MRU list
        sm->openSession(terminal, address);

        // It's a named session, so can save it
        ui->actionSave_Session->setEnabled(true);
    }
}

/**
 * @brief   MainWindow::~MainWindow - destructor
 *
 * @details Delete any objects obtained via 'new'.
 */
MainWindow::~MainWindow()
{
    //FIXME: delete of other objects obtained with 'new'
    delete ui;
}

/**
 * @brief   MainWindow::menuConnect - connect to a host
 *
 * @details Connect to a host. This may be reopening a connection to the existing settings or
 *          it may be the first time. If it's the first time, prompt the user for an address.
 */
void MainWindow::menuConnect()
{
    if (activeSettings.getHostAddress().isEmpty())
    {
        settings->showForm();
    }

    if (!activeSettings.getHostAddress().isEmpty())
    {
        terminal->openConnection(activeSettings.getHostAddress());

        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);
    }
}

/**
 * @brief   MainWindow::menuDisconnect - disconnect from a host
 *
 * @details Disconnect from the host
 */
void MainWindow::menuDisconnect()
{
    terminal->closeConnection();

    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
}

/**
 * @brief   MainWindow::menuSessionPreferences - show the preferences dialog.
 *
 * @details Open the preferences dialog.
 */
void MainWindow::menuSessionPreferences()
{
    settings->showForm();
}

/**
 * @brief   MainWindow::menuColourTheme - open the Colour Themes dialog
 *
 * @details Open the Colour themes dialog, which allows the user to add, change and delete
 *          Colour Themes.
 */
void MainWindow::menuColourTheme()
{
    colourTheme.exec();
}

/**
 * @brief   MainWindow::menuKeyboardTheme - open the Keyboard Themes dialog
 *
 * @details Open the Keyboard Themes dialog, which allows the user to add, change and delete
 *          Keyboard Themes.
 */
void MainWindow::menuKeyboardTheme()
{
    keyboardTheme.exec();
}

/**
 * @brief   MainWindow::menuAbout - display the 'About' dialog.
 *
 * @details Show some details about Q3270
 */
void MainWindow::menuAbout()
{
    QDialog *about = new QDialog;
    Ui::About *ab = new Ui::About;
    ab->setupUi(about);

    QString v = QString("Version ").append(Q3270_VERSION);
    ab->VersionNumber->setText(v);

    about->exec();

    delete ab;
    delete about;
}

/**
 * @brief   MainWindow::enableConnectMenu - enable/disable the Connect menu
 * @param   state - true to enable, false to disable
 *
 * @details Signalled when the Settings object's address is updated
 */
void MainWindow::enableConnectMenu(bool state)
{
    ui->actionConnect->setEnabled(state);
}

/**
 * @brief   MainWindow::enableDisconnectMenu - enable the disconnect menu
 *
 * @details Signalled when the Terminal is connected
 */
void MainWindow::enableDisconnectMenu()
{
    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setDisabled(true);
}

/**
 * @brief   MainWindow::disableDisconnectMenu - disable the disconnect menu
 *
 * @details Signalled when the Terminal is disconnected
 */
void MainWindow::disableDisconnectMenu()
{
    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
}

/**
 * @brief   MainWindow::updateMRUlist - add an entry to the most recently used list
 * @param   address - the address to be added
 *
 * @details Add an entry to the Most Recently Used list. This may be an existing entry,
 *          in which case, delete it from where it was and add it to the top of the list.
 */
void MainWindow::updateMRUlist(QString address)
{
    // Clear existing MRU list
    ui->menuRecentSessions->clear();

    // Find if the entry was in the list
    int index = mruList.indexOf(address);

    // If it was, remove it
    if (index != -1)
    {
        mruList.removeAt(index);
    }

    // Add the entry to the start of the MRU list
    mruList.prepend(address);

    // Loop through the MRU list, for the maximum allowed MRU entries, or the number of MRU entries so far
    for(int i = 0; i < mruList.size() && i < maxMruCount; i++)
    {
        QString entry = QString::number(i + 1) + ". ";
        // If it's not a session entry, add 'Host' to the entry
        if (!mruList.at(i).startsWith("Session "))
        {
            entry += "Host ";
        }
        ui->menuRecentSessions->addAction(entry + mruList.at(i), this, [this]() { mruConnect(); } );
    }

    QSettings applicationSettings;

    applicationSettings.beginWriteArray("RecentlyUsed");
    for(int i = 0; i < mruList.size() && i < maxMruCount; i++)
    {
        applicationSettings.setArrayIndex(i);
        applicationSettings.setValue("Entry", mruList.at(i));
    }
    applicationSettings.endArray();
}

/**
 * @brief   MainWindow::menuQuit - exit the application
 *
 * @details Exit the application.
 */
void MainWindow::menuQuit()
{
    QApplication::quit();
}

/**
 * @brief   MainWindow::closeEvent - close the window
 * @param   event - the close event
 *
 * @details Called when the user clicks the Close Window button.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings applicationSettings;

    applicationSettings.setValue("mainwindowgeometry", saveGeometry());
    applicationSettings.setValue("mainwindowstate", saveState());

    applicationSettings.setValue("restoresessions", true);

    applicationSettings.beginGroup("sessions");
    applicationSettings.remove("");
    applicationSettings.endGroup();

    applicationSettings.beginWriteArray("sessions");

/*
    for (int i = 0; i < ui->mdiArea->subWindowList().size(); i++)
    {

        applicationSettings.setArrayIndex(i);

        QMdiSubWindow *t = ui->mdiArea->subWindowList().at(i);

        applicationSettings.setValue("geometry", t->saveGeometry());
        applicationSettings.setValue("address", ((Terminal *)t)->address());
    }
*/
    applicationSettings.endArray();

    event->accept();
}

/**
 * @brief   MainWindow::showEvent - resize the content when the window is shown
 * @param   event - the event
 *
 * @details Ensure that the content of the terminal fits inside the window when the window is
 *          displayed.
 */
void MainWindow::showEvent(QShowEvent *event)
{
    terminal->fit();
}

/**
 * @brief   MainWindow::resizeEvent - resize the content when the window is resized
 * @param   event - the event
 *
 * @details Ensure that the content of the terminal fits inside the window when the window is
 *          resized.
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    terminal->fit();
}

/*
void MainWindow::subWindowClosed(QObject *closedWindow)
{
    subWindow = 0;

    QList<QAction *> acts = sessionGroup->actions();

    for(int i = 0; i < acts.size(); i++)
    {
        sessionGroup->removeAction(acts.at(i));
        ui->menuWindow->removeAction(acts.at(i));
        delete acts.at(i);
    }

    QList<QMdiSubWindow *> windows = ui->mdiArea->subWindowList();

    for (int i = 0; i < windows.size(); i++)
    {
        Terminal *t = (Terminal *)windows.at(i);

        if (t != closedWindow)
        {

            t->setWindowTitle("Session " + QString::number(++subWindow) + " [" + t->address() + "]");

            QAction *act = new QAction("Session " + QString::number(subWindow));

            act->setActionGroup(sessionGroup);
            act->setCheckable(true);
            act->setChecked(true);

            ui->menuWindow->addAction(act);

            connect(act, &QAction::triggered, t, [this, t, act]() { ui->mdiArea->setActiveSubWindow(t); act->setChecked(true); } );
        }
    }
}
*/

/*
 *
 * Desktop-type fade out notification - needs to be given its own class
 *
 *
 * QGraphicsOpacityEffect* effect=new QGraphicsOpacityEffect();
this->label->setGraphicsEffect(effect);
this->label->setStyleSheet("border: 3px solid gray;border-radius:20px;background-color:#ffffff;color:gray");
this->label->setAlignment(Qt::AlignCenter);
this->label->setText("Your Notification");
QPropertyAnimation* a=new QPropertyAnimation(effect,"opacity");
a->setDuration(1000);  // in miliseconds
a->setStartValue(0);
a->setEndValue(1);
a->setEasingCurve(QEasingCurve::InBack);
a->start(QPropertyAnimation::DeleteWhenStopped);
this->label->show();
connect(this->timer,&QTimer::timeout,this,&Notifier::fadeOut);
this->timer->start(2000); // 1000 ms to make the notification opacity full and 1000 seconds to call the fade out so total of 2000ms.

void fadeOut(){
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
    this->label->setGraphicsEffect(effect);
    QPropertyAnimation *a = new QPropertyAnimation(effect,"opacity");
    a->setDuration(1000); // it will took 1000ms to face out
    a->setStartValue(1);
    a->setEndValue(0);
    a->setEasingCurve(QEasingCurve::OutBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    connect(a,SIGNAL(finished()),this->label,SLOT(hide()));
}

*/
