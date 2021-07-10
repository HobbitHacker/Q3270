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

/**
 * Update address on form from supplied newAddress
 */
void Host::updateAddress(QString newAddress)
{

    // Determine if the supplied address contains an '@' denoting the LU to be used.
    //
    // Addresses can be of the form:
    //
    //   0700@localhost:3270
    //   console@1.2.3.4:23
    //   1.2.3.4:23

    if (newAddress.contains("@"))
    {
        ui->HostLU->setText(newAddress.section("@", 0, 0));
        ui->HostName->setText(newAddress.section("@", 1, 1).section(":", 0, 0));
        ui->HostPort->setText(newAddress.section(":", 1, 1));
    }
    else
    {
        ui->HostLU->setText("");
        ui->HostName->setText(newAddress.section(":", 0, 0));
        ui->HostPort->setText(newAddress.section(":", 1, 1));
    }

    hostName = ui->HostName->text();
    luName = ui->HostLU->text();
    port = ui->HostPort->text().toInt();
}
