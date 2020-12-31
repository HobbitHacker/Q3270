#include "ui_mainwindow.h"
#include "mainwindow.h"

//NOTE: SocketConnect will need to be created multiple times for multi-session support.

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new(Ui::MainWindow))
{
    ui->setupUi(this);

    QCoreApplication::setOrganizationDomain("styles.homeip.net");
    QCoreApplication::setApplicationName("Q3270");
    QCoreApplication::setOrganizationName("andyWare");

    applicationSettings = new QSettings();

    //TODO will need a new Terminal() for each tab
    t = new TerminalTab(ui->verticalLayout, applicationSettings);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::menuConnect()
{

    t->openConnection();

    ui->actionDisconnect->setEnabled(true);
    ui->actionConnect->setDisabled(true);
    ui->actionSet_Font->setEnabled(true);

}

void MainWindow::menuDisconnect()
{
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
        fs = new FontSelection(this,applicationSettings->value("font/name").toString(),applicationSettings->value("font/style").toString(),applicationSettings->value("font/size").toInt());
    }
    else
    {
        fs = new FontSelection(this,"ibm3270","Regular",8);
    }

    connect(fs, &FontSelection::setFont, this, &MainWindow::setSetting);

    if (fs->exec() == QDialog::Accepted)
    {
        t->setFont(fs->getFont());
        t->setScaleFont(fs->getScaling());
        //ui->verticalLayout->update();
    }
}

void MainWindow::menuTerminalSettings()
{
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
