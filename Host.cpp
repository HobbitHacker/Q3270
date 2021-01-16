#include "Host.h"
#include "ui_Host.h"

Host::Host(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Host)
{
    ui->setupUi(this);
}

Host::~Host()
{
    delete ui;
}

void Host::accept()
{
    QDialog::accept();
}

void Host::fieldsChanged()
{
    bool nameok = false;
    bool portok = false;;

    if (ui->HostName->text().length() > 0)
    {
        hostName = ui->HostName->text();
        nameok = true;
    }

    if (ui->HostPort->text().length() > 0)
    {
        port = ui->HostPort->text().toInt();
        if (port != 0)
        {
            portok = true;
        }
    }

    if (ui->HostLU->text().length() > 0)
    {
        luName = ui->HostLU->text();
    }

    if (nameok && portok)
    {
        ui->connectButton->setEnabled(true);
    }
    else
    {
        ui->connectButton->setDisabled(true);
    }
}
