#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "ui_About.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new(Ui::MainWindow))
{
    QCoreApplication::setOrganizationDomain("styles.homeip.net");
    QCoreApplication::setApplicationName("Q3270");
    QCoreApplication::setOrganizationName("andyWare");

    ui->setupUi(this);

    QSettings applicationSettings;

    sessionGroup = new QActionGroup(this);

    // Host connection dialog
    connectHost = new Host();

    // Session number
    subWindow = 0;

    restoreGeometry(applicationSettings.value("mainwindowgeometry").toByteArray());
    restoreState(applicationSettings.value("mainwindowstate").toByteArray());

    if (applicationSettings.contains("mrumax"))
    {
        maxMruCount = applicationSettings.value("mrumax").toInt();
    }
    else
    {
        maxMruCount = 10;
        applicationSettings.setValue("mrumax", 10);
    }

    int mruCount = applicationSettings.beginReadArray("mrulist");

    for(int i = 0; i < mruCount; i++)
    {
        applicationSettings.setArrayIndex(i);

        QString address = applicationSettings.value("address").toString();
        mruList.append(address);
        ui->menuRecentSessions->addAction(address, this, &MainWindow::mruConnect);
    }

    applicationSettings.endArray();

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

        ui->actionDisconnect->setEnabled(true);
        ui->actionReconnect->setDisabled(true);
        ui->actionConnect->setDisabled(true);

        applicationSettings.endArray();

    }

    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateMenuEntries);
}

void MainWindow::menuNew()
{
    newTab();
    menuConnect();
}

void MainWindow::mruConnect()
{
    QAction *sender = (QAction *)QObject::sender();

    QString address = sender->text();

    TerminalTab *t = newTab();

    t->openConnection(address);

    ui->actionDisconnect->setEnabled(true);
    ui->actionReconnect->setDisabled(true);
    ui->actionConnect->setDisabled(true);

    updateMRUlist(address);
}

TerminalTab *MainWindow::newTab()
{
    if (subWindow == 0)
    {
        ui->menuWindow->addSeparator()->setText(" ");
    }

    TerminalTab *t = new TerminalTab();

    ui->mdiArea->addSubWindow(t);

    t->show();
    t->setWindowTitle("Session " + QString::number(++subWindow));

    ui->actionTerminalSettings->setEnabled(true);

    QAction *act = new QAction("Session " + QString::number(subWindow));

    act->setActionGroup(sessionGroup);
    act->setCheckable(true);
    act->setChecked(true);

    ui->menuWindow->addAction(act);

    connect(act, &QAction::triggered, t, [this, t, act]() { ui->mdiArea->setActiveSubWindow(t); act->setChecked(true); } );
    connect(t, &TerminalTab::connectionClosed, this, &MainWindow::updateMenuEntries);

    return t;
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
        TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());
        t->openConnection(connectHost->hostName, connectHost->port, connectHost->luName);
        updateMRUlist((connectHost->luName.compare("") ? connectHost->luName + "@" : "") + connectHost->hostName + ":" + QString::number(connectHost->port));
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);
        ui->actionReconnect->setDisabled(true);
    }
}

void MainWindow::menuReconnect()
{
    TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());

    t->connectSession();

    ui->actionConnect->setDisabled(true);
    ui->actionDisconnect->setEnabled(true);
    ui->actionReconnect->setDisabled(true);
}

void MainWindow::menuDisconnect()
{
    TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());

    t->closeConnection();

    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
    ui->actionReconnect->setEnabled(true);
}

void MainWindow::menuTerminalSettings()
{
    TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());

    if (t)
    {
        t->showForm();
    }

    fflush(stdout);
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

void MainWindow::menuTabbedView(bool tabView)
{
    if (tabView)
    {
        ui->mdiArea->setViewMode(QMdiArea::TabbedView);
    }
    else
    {
        ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
    }
}

void MainWindow::updateMenuEntries()
{
    if(TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow()))
    {

        ui->actionTerminalSettings->setEnabled(true);
        if (t->view->connected)
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
    else
    {
        ui->actionDisconnect->setDisabled(true);
        ui->actionConnect->setDisabled(true);
        ui->actionTerminalSettings->setDisabled(true);
    }
}

void MainWindow::updateMRUlist(QString address)
{
    ui->menuRecentSessions->clear();

    int x = mruList.indexOf(address);

    if (x != -1)
    {
        mruList.removeAt(x);
    }

    mruList.prepend(address);

    for(int i = 0; i < mruList.size() && i < maxMruCount; i++)
    {
        ui->menuRecentSessions->addAction(mruList.at(i), this, [this]() { mruConnect(); } );
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
    disconnect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateMenuEntries);
    disconnect(this, "&updateMenuEntries()");

    QSettings applicationSettings;

    applicationSettings.setValue("mainwindowgeometry", saveGeometry());
    applicationSettings.setValue("mainwindowstate", saveState());

    applicationSettings.setValue("restoresessions", true);

    applicationSettings.beginWriteArray("sessions");

    for (int i = 0; i < ui->mdiArea->subWindowList().size(); i++)
    {

        applicationSettings.setArrayIndex(i);

        QMdiSubWindow *t = ui->mdiArea->subWindowList().at(i);

        applicationSettings.setValue("geometry", t->saveGeometry());
        applicationSettings.setValue("address", ((TerminalTab *)t)->address());
    }

    applicationSettings.endArray();
}
