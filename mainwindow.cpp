#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "ui_About.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new(Ui::MainWindow))
{
    QCoreApplication::setOrganizationDomain("styles.homeip.net");
    QCoreApplication::setApplicationName("Q3270");
    QCoreApplication::setOrganizationName("andyWare");

    ui->setupUi(this);

    applicationSettings = new QSettings();

    restoreGeometry(applicationSettings->value("mainwindowgeometry").toByteArray());
    restoreState(applicationSettings->value("mainwindowstate").toByteArray());

    if (applicationSettings->contains("mrumax"))
    {
        maxMruCount = applicationSettings->value("mrumax").toInt();
    }
    else
    {
        maxMruCount = 10;
        applicationSettings->setValue("mrumax", 10);
    }

    int mruCount = applicationSettings->beginReadArray("mrulist");

    for(int i = 0; i < mruCount; i++)
    {
        applicationSettings->setArrayIndex(i);

        QString address = applicationSettings->value("address").toString();
        mruList.append(address);
        ui->menuRecentSessions->addAction(address, this, &MainWindow::mruConnect);
    }

    applicationSettings->endArray();

    connect(ui->mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::updateMenuEntries);

    sessionGroup = new QActionGroup(this);

    // Host connection dialog
    connectHost = new Host();

    subWindow = 0;
}

void MainWindow::menuNew()
{
    newTab();
    menuConnect();
}

void MainWindow::mruConnect()
{
    QAction *sender = (QAction *)QObject::sender();

    QString menuText = sender->text();

    QString host;
    int port;
    QString luname;

    if (menuText.contains("@"))
    {
       luname = menuText.section("@", 0, 0);
       host = menuText.section("@", 1, 1).section(":", 0, 0);
       port = menuText.section(":", 1, 1).toInt();
    }
    else
    {
        luname = "";
        host = menuText.section(":", 0, 0);
        port = menuText.section(":", 1, 1).toInt();
    }

    TerminalTab *t = newTab();
    t->openConnection(host, port, luname);

    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setDisabled(true);

    updateMRUlist(menuText);
}

TerminalTab *MainWindow::newTab()
{
    TerminalTab *t = new TerminalTab();
    ui->mdiArea->addSubWindow(t);
    t->show();
    ui->actionTerminalSettings->setEnabled(true);

    if (subWindow == 0)
    {
        ui->menuWindow->addSeparator();
    }

    QAction *act = new QAction("Session " + QString::number(++subWindow));
    act->setActionGroup(sessionGroup);
    act->setCheckable(true);
    act->setChecked(true);
    ui->menuWindow->addAction(act);
    connect(act, &QAction::triggered, t, [this, t, act]() { ui->mdiArea->setActiveSubWindow(t); act->setChecked(true); } );
    connect(t, &TerminalTab::connectionClosed, this, &MainWindow::updateMenuEntries);

    t->setWindowTitle("Session " + QString::number(subWindow));

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

    applicationSettings->beginWriteArray("mrulist");
    for(int i = 0; i < mruList.size() && i < maxMruCount; i++)
    {
        applicationSettings->setArrayIndex(i);
        applicationSettings->setValue("address", mruList.at(i));
    }
    applicationSettings->endArray();
}

void MainWindow::menuQuit()
{
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    applicationSettings->setValue("mainwindowgeometry", saveGeometry());
    applicationSettings->setValue("mainwindowstate", saveState());
}

void MainWindow::setSetting(QString key, QString value)
{
    applicationSettings->setValue(key, value);
    printf("Saving %s as %s\n", key.toLatin1().data(), value.toLatin1().data());
    fflush(stdout);
}
