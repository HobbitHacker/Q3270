#include "ui_MainWindow.h"
#include "MainWindow.h"
#include "ui_About.h"

MainWindow::MainWindow(QString sessionName) : QMainWindow(nullptr), ui(new(Ui::MainWindow))
{
    QCoreApplication::setOrganizationDomain("styles.homeip.net");
    QCoreApplication::setApplicationName("Q3270");
    QCoreApplication::setOrganizationName("andyWare");

    ui->setupUi(this);

    // Read global settings
    QSettings settings;

    settings.beginGroup("Global");

    // Most-recently used; default to 10
    maxMruCount = settings.value("MRUMax", 10).toInt();

    // Most-recently used list; might be addresses or sessions
    settings.beginGroup("MRUList");

    // List of all entries
    QStringList savedMRUList = settings.childKeys();

    for(int i = 0; i < savedMRUList.count(); i++)
    {

        // Store "Address" or "Session" keys
        if (!savedMRUList.at(i).compare("Address"))
        {
            QString thisAddress = QString::number(i + 1) + ". Host " + settings.value("Address").toString();
            mruList.append(thisAddress);
            ui->menuRecentSessions->addAction(thisAddress, this, &MainWindow::mruConnect);
        }
        else if (!savedMRUList.at(i).compare("Session"))
        {
            QString thisAddress = QString::number(i + 1) + ". Session " + settings.value("Session").toString();
            mruList.append(thisAddress);
            ui->menuRecentSessions->addAction(thisAddress, this, &MainWindow::mruConnect);
        }
    }

    settings.endGroup();


    sessionGroup = new QActionGroup(this);

    // Host connection dialog
    connectHost = new Host();

    // Colour theme dialog
    colourTheme = new ColourTheme();

    // Keyboard theme dialog
    keyboardTheme = new KeyboardTheme();

    restoreGeometry(settings.value("mainwindowgeometry").toByteArray());
    restoreState(settings.value("mainwindowstate").toByteArray());

/*
    if (applicationSettings.value("restorewindows", false).toBool())
    {
        int sessionCount = applicationSettings.beginReadArray("sessions");
        for (int i = 0; i < sessionCount; i++)
        {
            applicationSettings.setArrayIndex(i);

            TerminalTab *t = newTab();
            t->restoreGeometry(applicationSettings.value("geometry").toByteArray());
            t->openConnection(applicationSettings.value("address").toString());
        }

        applicationSettings.endArray();

    }
*/

    // Set defaults for Connect options
    ui->actionDisconnect->setDisabled(true);
    ui->actionReconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);

    terminal = new TerminalTab(ui->terminalLayout, colourTheme, keyboardTheme, sessionName);
}

void MainWindow::menuNew()
{
    menuConnect();
}

void MainWindow::menuSaveSession()
{
    // Save session dialog
    SessionManagement sm;

    sm.saveSession(terminal);
}

void MainWindow::menuOpenSession()
{
    // Open session dialog
    SessionManagement sm;

    sm.openSession(terminal);
}

void MainWindow::mruConnect()
{
    // Find the sending object - the line in the 'Recent' menu
    QAction *sender = (QAction *)QObject::sender();

    // Will contain either session name or host address
    QString address;

    // Split the menu entry into parts: 1. Host 192.168.0.1:23 or 1. Session Fred
    QStringList parts = sender->text().split(" ");

    // If this is a Host address, use that to connect to
    if (!parts[1].compare("Host"))
    {
        // Pick up address and connect
        address = parts[2];
        terminal->openConnection(address);

        // Set Connect menu entries
        ui->actionDisconnect->setEnabled(true);
        ui->actionReconnect->setDisabled(true);
        ui->actionConnect->setDisabled(true);
    }
    else
    {
        // It's a session name, so concatenate the remaining parts of the menu entry
        for(int i = 2; i < parts.count(); i++)
        {
            address = address.append(parts[i] + " ");
        }
    }

        updateMRUlist(terminal->address());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete connectHost;
}

void MainWindow::menuConnect()
{

    if (connectHost->exec() == QDialog::Accepted)
    {
        terminal->openConnection(connectHost->hostName, connectHost->port, connectHost->luName);
        updateMRUlist((connectHost->luName.compare("") ? connectHost->luName + "@" : "") + connectHost->hostName + ":" + QString::number(connectHost->port));
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);
        ui->actionReconnect->setDisabled(true);
    }
}

void MainWindow::menuReconnect()
{
    terminal->connectSession();

    ui->actionConnect->setDisabled(true);
    ui->actionDisconnect->setEnabled(true);
    ui->actionReconnect->setDisabled(true);
}

void MainWindow::menuDisconnect()
{
    terminal->closeConnection();

    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
    ui->actionReconnect->setEnabled(true);
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
    QDialog *about = new QDialog(0,0);
    Ui::About *ab = new Ui::About;
    ab->setupUi(about);

    QString v = QString("Version ").append(Q3270_VERSION);
    ab->VersionNumber->setText(v);

    about->exec();

    delete ab;
    delete about;
}

void MainWindow::updateMenuEntries()
{
    if (terminal->view->connected)
    {
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);
        ui->actionReconnect->setDisabled(true);
    }
    else
    {
        ui->actionReconnect->setEnabled(true);
        ui->actionDisconnect->setDisabled(true);
        ui->actionConnect->setEnabled(true);
    }
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
        if (mruList.at(i).startsWith("Session "))
        {
            entry += "Session ";
        }
        else
        {
            entry += "Host ";
        }
        ui->menuRecentSessions->addAction(entry + mruList.at(i), this, [this]() { mruConnect(); } );
    }

    QSettings applicationSettings;

    applicationSettings.beginWriteArray("mrulist");
    for(int i = 0; i < mruList.size() && i < maxMruCount; i++)
    {
        applicationSettings.setArrayIndex(i);
        applicationSettings.setValue("address", mruList.at(i));
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
