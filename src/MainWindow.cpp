/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "MainWindow.h"

#include "ui_MainWindowDialog.h"
#include "ui_About.h"
#include "ui_CertificateDetails.h"
#include "HostAddressUtils.h"
#include "Sessions/ManageAutoStartDialog.h"

#include "Sessions/SaveSessionDialog.h"
#include "Sessions/OpenSessionDialog.h"
#include "Sessions/ManageSessionsDialog.h"

#include "Version.h"
#include "Q3270.h"
#include "Models/Session.h"
/**
 * @brief   MainWindow::MainWindow - the main application
 * @param   launchParms - a name
 *
 * @details Initialise the application, and open any session name passed to it. The session
 *          is a struct that contains an existing MainWindow or null if this is the first one,
 *          and any session name to be opened.
 */
MainWindow::MainWindow(MainWindow::LaunchParms launchParms) : QMainWindow(nullptr),
    sessionStore(),
    keyboardStore(),
    keyboardTheme(nullptr),
    settings(nullptr),
    colourTheme(nullptr)
{   
    QCoreApplication::setApplicationVersion(Q3270_VERSION_FULL);

    ui = new Ui::MainWindowDialog;

    ui->setupUi(this);

    connect(ui->actionSessionPreferences,        &QAction::triggered, this, &MainWindow::menuSessionPreferences);
    connect(ui->actionConnect,                   &QAction::triggered, this, &MainWindow::menuConnect);
    connect(ui->actionQuit,                      &QAction::triggered, this, &MainWindow::menuQuit);
    connect(ui->actionDisconnect,                &QAction::triggered, this, &MainWindow::menuDisconnect);
    connect(ui->actionNew,                       &QAction::triggered, this, &MainWindow::menuNew);
    connect(ui->actionAbout_Q3270,               &QAction::triggered, this, &MainWindow::menuAbout);
    connect(ui->actionColourThemes,              &QAction::triggered, this, &MainWindow::menuColourTheme);
    connect(ui->actionSave_SessionAs,            &QAction::triggered, this, &MainWindow::menuSaveSessionAs);
    connect(ui->actionOpen_Session,              &QAction::triggered, this, &MainWindow::menuOpenSession);
    connect(ui->actionKeyboardThemes,            &QAction::triggered, this, &MainWindow::menuKeyboardTheme);
    connect(ui->actionOpen_Duplicate_Session,    &QAction::triggered, this, &MainWindow::menuDuplicate);
    connect(ui->actionManage_Sessions,           &QAction::triggered, this, &MainWindow::menuManageSessions);
    connect(ui->actionManage_Auto_Sart_Sessions, &QAction::triggered, this, &MainWindow::menuManageAutostartSessions);
    connect(ui->actionSave_Session,              &QAction::triggered, this, &MainWindow::menuSaveSession);
    connect(ui->actionConnection_Information,    &QAction::triggered, this, &MainWindow::menuAboutConnection);

    connect(&activeSettings, &ActiveSettings::keyboardThemeChanged, this, &MainWindow::activeKeyboardNameChanged);
    connect(&activeSettings, &ActiveSettings::colourThemeChanged,   this, &MainWindow::activeColoursNameChanged);

    // Get Sessions from settings
    sessionStore.load();

    populateMRU();

    // Preferences dialog
    settings = new PreferencesDialog(codePage, activeSettings, keyboardStore, colourStore);

    // Change Connect menu entry when connected, or when user fills in hostname field in Settings
    connect(settings, &PreferencesDialog::connectValid, this, &MainWindow::enableConnectMenu);

    // Construct the keyboard and colour mapping dialogs
    keyboardTheme = new KeyboardThemeDialog(keyboardStore);
    colourTheme = new ColourTheme(colourStore);

    // Construct the 3270 Terminal
    terminal = new Terminal(ui->terminalLayout, activeSettings, codePage, keyboard, Colours::getFactoryTheme());

    // Used for dynamically showing font changes when using the font selection dialog
    connect(settings, &PreferencesDialog::tempFontChange, terminal, &Terminal::setCurrentFont);

    // Check if the user modified the active colour & keyboard themes through the theme dialogs
    connect(keyboardTheme, &KeyboardThemeDialog::themesApplied, this, &MainWindow::checkKeyboardThemeModified);
    connect(colourTheme, &ColourTheme::themesApplied, this, &MainWindow::checkColourThemeModified);

    // Enable/Disable menu entries if connected/disconnected
    connect(terminal, &Terminal::disconnected, this, &MainWindow::disableDisconnectMenu);
    connect(terminal, &Terminal::disconnected, settings, &PreferencesDialog::disconnected);
    
    connect(terminal, &Terminal::connectionEstablished, this, &MainWindow::enableDisconnectMenu);
    connect(terminal, &Terminal::connectionEstablished, settings, &PreferencesDialog::connected);

    // Set defaults for Connect options
    ui->actionDisconnect->setDisabled(true);

/*
    // If a session name was passed to the MainWindow, restore the window size/position
    // and open it
    if (!launchParms.session.isEmpty())
    {
        //TODO: Decide if Window Geometry is just going to mean that a session always has the same pos etc on screen
        //TODO: Use QWidget.resize and QWidget.move instead. See QSettings doc
        restoreGeometry(savedSettings.value(launchParms.session + "/WindowGeometry").toByteArray());
        //sm->openSession(terminal, QUrl::fromPercentEncoding(launchParms.session.toLatin1()));

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
    if (launchParms.mw == nullptr)
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
                // sm->openSession(terminal, QUrl::fromPercentEncoding(savedSettings.value("Session").toString().toLatin1()));

                // Enable Save Session menu entry as it's a named session
                ui->actionSave_Session->setEnabled(true);

                // Disable/Enable Reconnect etc menu entries
                ui->actionDisconnect->setEnabled(true);
            }
        }

        savedSettings.endArray();
    } */
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
    MainWindow *newWindow = new MainWindow({ this, activeSettings.getSessionName() });
    newWindow->show();
}

/**
 * @brief   MainWindow::menuSaveSession - save the current session
 *
 * @details Save the current session settings to the config file.
 */
void MainWindow::menuSaveSession()
{
    sessionStore.saveSession(::Session::fromActiveSettings(activeSettings));
}

/**
 * @brief   MainWindow::menuSaveSessionAs - save current session as a new name
 *
 * @details Save the current session settings to the config file with a new session name.
 */
void MainWindow::menuSaveSessionAs()
{
    // Save Session dialog
    SaveSessionDialog dlg(sessionStore, activeSettings, this);

    bool saved = dlg.exec();

    // If the 'Save Session' option was disabled and a session is now saved, enable the menu option
    if (!ui->actionSave_Session->isEnabled())
        ui->actionSave_Session->setEnabled(saved);
}

/**
 * @brief   MainWindow::menuOpenSession - open a session
 *
 * @details Open a session
 */
void MainWindow::menuOpenSession()
{
    // Open Session dialog
    OpenSessionDialog dlg(sessionStore, activeSettings, this);

    bool opened(dlg.exec());

    if (opened == QDialog::Accepted)
    {
        terminal->connectSession();
        updateMRUList();
    }
}

/**
 * @brief   MainWindow::menuManageSessions - Open the manage session dialog
 *
 * @details Open the manage sessions dialog to allow the user to add/delete sessions.
 */
void MainWindow::menuManageSessions()
{
    ManageSessionsDialog dlg(sessionStore, this);
    dlg.exec();
}

/**
 * @brief   MainWindow::menuManageSessions - Open the manage session dialog
 *
 * @details Open the manage sessions dialog to allow the user to add/delete sessions.
 */
void MainWindow::menuManageAutostartSessions()
{
    // Manage Sessions Dialog

    SessionStore s;
    ManageAutoStartDialog dlg(s);
    dlg.exec();
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
    if (terminal->isConnected())
    {
        terminal->closeConnection();
    }
    // Find the sending object - the line in the 'Recent' menu
    QAction *sender = (QAction *)QObject::sender();

    // Split the menu entry into parts: 1. Host 192.168.0.1:23 or 1. Session Fred
    QStringList parts = sender->text().split(" ");

    qDebug() << parts[1];
    qDebug() << parts[2];

    // If this is a Host address, use that to connect to
    if (!parts[1].compare("Host"))
    {
        // Store the address in the current settings
        QString namePart;
        int portPart;
        QString LUPart;

        HostAddressUtils::parse(parts[2], namePart, portPart, LUPart);

        activeSettings.setHostAddress(namePart, portPart, LUPart);

        // Pick up address and connect
        terminal->connectSession();

        // Set Connect menu entries
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);

        // Not a named session, so can't save it as one (use Save Session As.. instead)
        ui->actionSave_Session->setDisabled(true);

        // Update recently used list
        updateMRUList();
    }
    else
    {
        // Will contain either session name or host address
        Session s = sessionStore.getSession(parts.mid(2).join(" "));

        s.toActiveSettings(activeSettings);

        // Connect the session
        terminal->connectSession();

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
    // Cleanup
    delete keyboardTheme;
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
        // TODO: DRY - these two lines are duplicated from menuOpenSession()
        terminal->connectSession();
        updateMRUList();
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
    colourTheme->exec();
}

/**
 * @brief   MainWindow::menuKeyboardTheme - open the Keyboard Themes dialog
 *
 * @details Open the Keyboard Themes dialog, which allows the user to add, change and delete
 *          Keyboard Themes.
 */
void MainWindow::menuKeyboardTheme()
{
/*    if (keyboardTheme)
    {
        const QString kbTheme = activeSettings.getKeyboardThemeName();
        if (!kbTheme.isEmpty())
            keyboardTheme->getTheme(kbTheme);
*/
    keyboardTheme->exec();
  //  }
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

    ab->logo->load(QStringLiteral(":/Icons/q3270.svg"));

    ab->versionQ3270->setText(QString("Version ").append(Q3270_VERSION));
    ab->versionQt->setText(QStringLiteral(QT_VERSION_STR));

    QString distro;

    QFile f("/etc/os-release");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&f);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("PRETTY_NAME=")) {
                distro = line.section('=', 1).remove('"').trimmed();
                break;
            }
        }
        ab->OSName->setText(distro);
    }

    about->exec();

    delete ab;
    delete about;
}

/**
 * @brief   MainWindow::menuAboutConnection - display the 'About Connection' dialog.
 *
 * @details Show some details about the connection to the host
 */
void MainWindow::menuAboutConnection()
{
    CertificateDetails certDetails(terminal->getCertDetails());
    certDetails.exec();
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
    ui->actionConnection_Information->setDisabled(true);
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
    ui->actionConnection_Information->setEnabled(true);
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
    ui->actionConnection_Information->setEnabled(true);
}

void MainWindow::populateMRU()
{
    // Read global settings
    QSettings savedSettings(Q3270_ORG, Q3270_APP);

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
}


/**
 * @brief   MainWindow::updateMRUlist - add an entry to the most recently used list
 * @param   address - the address to be added
 *
 * @details Add an entry to the Most Recently Used list. This may be an existing entry,
 *          in which case, delete it from where it was and add it to the top of the list.
 */
void MainWindow::updateMRUList()
{
    QString address = activeSettings.getSessionName().isEmpty() ? QString("Host ").append(activeSettings.getHostAddress()) : QString("Session ").append(activeSettings.getSessionName());

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

   QSettings applicationSettings(Q3270_ORG, Q3270_APP);

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

#if 0

// TODO: Deliberately disabled for later when address process is sorted.
    QSettings applicationSettings(Q3270_ORG, Q3270_APP);

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
#endif

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

void MainWindow::checkKeyboardThemeModified(const QString &name)
{
    // Update the keyboard map when the user changes the active one
    if (activeSettings.getKeyboardThemeName() == name)
        activeKeyboardNameChanged(name);
}

void MainWindow::activeKeyboardNameChanged(const QString &name)
{
    // Update the keyboard
    KeyboardMap km = keyboardStore.getTheme(name);
    keyboard.setMap(km);
}

void MainWindow::checkColourThemeModified(const QString &name)
{
    // Update the colours when the user changes the active one
    if (activeSettings.getColourThemeName() == name)
        activeColoursNameChanged(name);
}

void MainWindow::activeColoursNameChanged(const QString &name)
{
    // Update the colours
    Colours cs = colourStore.getTheme(name);
    terminal->setColourTheme(cs);
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
