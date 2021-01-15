#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new(Ui::MainWindow))
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationDomain("styles.homeip.net");
    QCoreApplication::setApplicationName("Q3270");
    QCoreApplication::setOrganizationName("andyWare");

    applicationSettings = new QSettings();

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
}

void MainWindow::menuNew()
{
    TerminalTab *t = new TerminalTab(applicationSettings);
    ui->mdiArea->addSubWindow(t);
    t->show();
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

    TerminalTab *t = new TerminalTab(applicationSettings);
    ui->mdiArea->addSubWindow(t);
    t->show();
    t->openConnection(host, port, luname);

    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setDisabled(true);
    ui->actionSet_Font->setEnabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::menuConnect()
{
    Host *h = new Host();

    if (h->exec() == QDialog::Accepted)
    {
        TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());
        t->openConnection(h->hostName, h->port, h->luName);
        updateMRUlist(h->luName.compare("") ? h->luName + "@" : "" + h->hostName + ":" + QString::number(h->port));
        ui->actionDisconnect->setEnabled(true);
        ui->actionConnect->setDisabled(true);
        ui->actionSet_Font->setEnabled(true);
    }
}

void MainWindow::menuDisconnect()
{
    TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());

    t->closeConnection();

    ui->actionDisconnect->setDisabled(true);
    ui->actionConnect->setEnabled(true);
    ui->actionSet_Font->setDisabled(true);
}

void MainWindow::menuSetFont()
{
    FontSelection *fs;

    if (applicationSettings->contains("font/name"))
    {
        fs = new FontSelection(this,applicationSettings->value("font/name").toString(),applicationSettings->value("font/style").toString(),applicationSettings->value("font/size").toInt(), applicationSettings->value("font/scale").toBool());
    }
    else
    {
        fs = new FontSelection(this,"ibm3270","Regular",8);
    }

    connect(fs, &FontSelection::setFont, this, &MainWindow::setSetting);

    if (fs->exec() == QDialog::Accepted)
    {
        TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());
        t->setFont(fs->getFont());
        t->setScaleFont(fs->getScaling());
        setSetting("font/scale", QString::number(fs->getScaling()));
        //ui->verticalLayout->update();
    }
}

void MainWindow::menuTerminalSettings()
{
    TerminalTab *t = (TerminalTab *)(ui->mdiArea->activeSubWindow());

    Settings *s = new Settings(this, t);

    if (s->exec() == QDialog::Accepted)
    {
        setSetting("terminal/model", t->name());
        setSetting("terminal/height",QString::number(t->terminalHeight()));
        setSetting("terminal/width", QString::number(t->terminalWidth()));
        setSetting("terminal/cursorblink", QString::number(t->view->getBlink()));
        setSetting("terminal/cursorblinkspeed", QString::number(t->view->getBlinkSpeed()));
    }

    fflush(stdout);
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

        if (t->view->connected)
        {
            ui->actionDisconnect->setEnabled(true);
            ui->actionConnect->setDisabled(true);
            ui->actionSet_Font->setEnabled(true);
        }
        else
        {
            ui->actionDisconnect->setDisabled(true);
            ui->actionConnect->setEnabled(true);
            ui->actionSet_Font->setDisabled(true);
        }
    }
    else
    {
        ui->actionDisconnect->setDisabled(true);
        ui->actionConnect->setDisabled(true);
        ui->actionSet_Font->setDisabled(true);
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

    for(int i = 0; i <= mruList.size(); i++)
    {
        ui->menuRecentSessions->addAction(mruList.at(i), this, [this]() { mruConnect(); } );
    }

    applicationSettings->beginWriteArray("mrulist");
    for(int i = 0; i <= mruList.size(); i++)
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

void MainWindow::setSetting(QString key, QString value)
{
    applicationSettings->setValue(key, value);
    printf("Saving %s as %s\n", key.toLatin1().data(), value.toLatin1().data());
    fflush(stdout);
}
