#include "ui_MainWindow.h"
#include "MainWindow.h"
#include "ui_About.h"

MainWindow::MainWindow(MainWindow::Session s) : QMainWindow(nullptr), ui(new(Ui::MainWindow))
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

    // Colour theme dialog
    colourTheme = new ColourTheme();

    // Keyboard theme dialog
    keyboardTheme = new KeyboardTheme();

    // Create Settings object
    settings = new Settings(colourTheme, keyboardTheme);

    // Session Management dialog
    sm = new SessionManagement(settings);

    // Update MRU entries if the user opens a session
    connect(sm, &SessionManagement::sessionOpened, this, &MainWindow::updateMRUlist);

    // Change Connect menu entry when connected, or when user fills in hostname field in Settings
    connect(settings, &Settings::connectValid, this, &MainWindow::enableConnectMenu);

    // Set defaults for Connect options
    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setDisabled(true);

    terminal = new TerminalTab(ui->terminalLayout, settings, colourTheme, keyboardTheme, s.session);

    // Refresh menu entries if connected/disconnected
    connect(terminal, &TerminalTab::disconnected, this, &MainWindow::disableDisconnectMenu);
    connect(terminal, &TerminalTab::connectionEstablished, this, &MainWindow::enableDisconnectMenu);

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

                // Update settings form with new address
                settings->setAddress(QUrl::fromPercentEncoding(savedSettings.value("Session").toString().toLatin1()));

                // Enable Save Session menu entry as it's a named session
                ui->actionSave_Session->setEnabled(true);

                // Disable/Enable Reconnect etc menu entries
                ui->actionDisconnect->setEnabled(true);
            }
        }

        savedSettings.endArray();
    }


}

void MainWindow::menuNew()
{
    MainWindow *newWindow = new MainWindow( { this, "" });
    newWindow->show();
}

void MainWindow::menuDuplicate()
{
    MainWindow *newWindow = new MainWindow({ this, terminal->getSessionName() });
    newWindow->show();
}

void MainWindow::menuSaveSession()
{
    sm->saveSettings();
}

void MainWindow::menuSaveSessionAs()
{
    // Save Session dialog, setting the Save Session menu entry dis/enabled
    ui->actionSave_Session->setEnabled(sm->saveSessionAs(terminal));
}

void MainWindow::menuOpenSession()
{
    // Open Session dialog
    ui->actionSave_Session->setEnabled(sm->openSession(terminal));
}

void MainWindow::menuManageSessions()
{
    // Manage Sessions Dialog
    sm->manageSessions();
}

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
        settings->setAddress(parts[2]);
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

MainWindow::~MainWindow()
{
    //FIXME: delete of other objects obtained with 'new'
    delete ui;
}

void MainWindow::menuConnect()
{
    if (settings->getAddress().isEmpty())
    {
        terminal->showForm();
    }

    if (!settings->getAddress().isEmpty())
    {
        terminal->openConnection(settings->getAddress());

        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);
    }
}

void MainWindow::menuDisconnect()
{
    terminal->closeConnection();

    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
}

void MainWindow::menuSessionPreferences()
{
    terminal->showForm();
}

void MainWindow::menuColourTheme()
{
    colourTheme->exec();
}

void MainWindow::menuKeyboardTheme()
{
    keyboardTheme->exec();
}

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

// Signalled when the Settings object's address is updated
void MainWindow::enableConnectMenu(bool state)
{
    ui->actionConnect->setEnabled(state);
}

// Signalled when the Terminal is connected
void MainWindow::enableDisconnectMenu()
{
    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setDisabled(true);
}

// Signalled when the Terminal is disconnected
void MainWindow::disableDisconnectMenu()
{
    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
}


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

void MainWindow::menuQuit()
{
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    disconnect(this, "&updateMenuEntries()");

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
        applicationSettings.setValue("address", ((TerminalTab *)t)->address());
    }
*/
    applicationSettings.endArray();

    event->accept();
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
        TerminalTab *t = (TerminalTab *)windows.at(i);

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
