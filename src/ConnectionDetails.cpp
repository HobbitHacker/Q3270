/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QDebug>

#include <QSslCertificateExtension>

#include "ConnectionDetails.h"
#include "ui_ConnectionDetails.h"

/**
 *  @brief   ConnectionDetails::ConnectionDetails - Display Certificate Details
 *
 *  @details This class displays details about certificates used for the connection. Certificates are
 *           extracted from the connection and stored in this class because each certificate is displayed
 *           individually
 */
ConnectionDetails::ConnectionDetails(Terminal *terminal, ActiveSettings &activeSettings, QWidget *parent) :
    QDialog(parent),
    terminal(terminal),
    activeSettings(activeSettings),
    ui(new Ui::ConnectionDetails)
{
    ui->setupUi(this);

    ui->hostname->setText(activeSettings.getHostName());
    ui->port->setText(QString::number(activeSettings.getHostPort()));
    ui->logicalunit->setText(activeSettings.getHostLU());

    ui->keyboardTheme->setText(activeSettings.getKeyboardThemeName());
    ui->colourTheme->setText(activeSettings.getColourThemeName());

    ui->session->setText(activeSettings.getSessionName());
    ui->description->setText(activeSettings.getDescription());

    certs = terminal->getCertDetails();
    ui->Details->setDisabled(certs.isEmpty());;

    if (certs.length() == 0)
    {
        ui->secure->setText("No");
    }
    else
    {
        ui->secure->setText("Yes");

        connect(ui->certificateList, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ConnectionDetails::showCertificate);

        ui->certificateList->clear();

        for (int i = 0; i < certs.size(); ++i)
        {
            const QSslCertificate &cert = certs.at(i);
            ui->certificateList->addItem (tr("%1%2 (%3)").arg(!i ? tr("This site: ") : tr("Issued by: "))
                                             .arg(cert.subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' ')))
                                             .arg(cert.subjectInfo(QSslCertificate::CommonName).join(QLatin1Char(' '))));
        }

        QSslCertificate intermediate = certs.last(); // issuer of the leaf

        // Find the root in Qt's CA database
        QList<QSslCertificate> cas = QSslConfiguration::defaultConfiguration().caCertificates();
        for (const QSslCertificate &ca : cas) {
            if (ca.subjectInfo(QSslCertificate::CommonName) == intermediate.issuerInfo(QSslCertificate::CommonName))
            {
                certs.append(ca);
                ui->certificateList->addItem (tr("%1%2 (%3)").arg(tr("Issued by: "))
                                                 .arg(ca.subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' ')))
                                                 .arg(ca.subjectInfo(QSslCertificate::CommonName).join(QLatin1Char(' '))));
                break;
            }
        }

        ui->certificateList->setCurrentIndex(0);

        font.setBold(true);
    }

    ui->tabWidget->setCurrentIndex(0);
}

/**
 * @brief   ConnectionDetails::~ConnectionDetails - destructor
 *
 * @details Delete the ui object created in the constructor.
 */
ConnectionDetails::~ConnectionDetails()
{
    delete ui;
}

/**
 * @brief   ConnectionDetails::showCertificate - show a specific certificate
 * @param   i - the index of the certificate in the drop-down list
 *
 * @details This is called when the user selects a different certificate from the drop-down list.
 *          The fields are added to the table in the dialog, and are only shown if they have content.
 *          (ie, blank fields are ignored).
 */
void ConnectionDetails::showCertificate(int i)
{
    ui->certificateDetails->setRowCount(0);

    if (i >= 0 && i < certs.size()) {
        const QSslCertificate &cert = certs.at(i);

        addRow(tr("Organisation"), cert.subjectInfo(QSslCertificate::Organization));
        addRow(tr("Subunit"), cert.subjectInfo(QSslCertificate::OrganizationalUnitName));
        addRow(tr("Country"), cert.subjectInfo(QSslCertificate::CountryName));
        addRow(tr("Locality"), cert.subjectInfo(QSslCertificate::LocalityName));
        addRow(tr("State/Province"), cert.subjectInfo(QSslCertificate::StateOrProvinceName));
        addRow(tr("Common Name"), cert.subjectInfo(QSslCertificate::CommonName));
        addRow(tr("Subject Alternative Name"), QStringList(cert.subjectAlternativeNames().values()));
        addRow(tr("Issuer Organization"), cert.issuerInfo(QSslCertificate::Organization));
        addRow(tr("Issuer Unit Name"), cert.issuerInfo(QSslCertificate::OrganizationalUnitName));
        addRow(tr("Issuer Country"), cert.issuerInfo(QSslCertificate::CountryName));
        addRow(tr("Issuer Locality"), cert.issuerInfo(QSslCertificate::LocalityName));
        addRow(tr("Issuer State/Province"), cert.issuerInfo(QSslCertificate::StateOrProvinceName));
        addRow(tr("Issuer Common Name"), cert.issuerInfo(QSslCertificate::CommonName));
        addRow(tr("Valid From"), QStringList(QLocale().toString(cert.effectiveDate().toLocalTime(), QLocale::LongFormat)));
        addRow(tr("Valid Until"),QStringList(QLocale().toString(cert.expiryDate().toLocalTime(), QLocale::LongFormat)));

        for (int j = 0; j < cert.extensions().size(); ++j) {
            QSslCertificateExtension ext = cert.extensions().at(j);
            addRow(tr("X509 ") + ext.name(), QStringList(ext.value().toString()));
        }
//        addRow(tr("MD5 Digest"), cert.digest().to));
    }

    ui->certificateDetails->resizeRowsToContents();
}

/**
 * @brief   ConnectionDetails::addRow - add a row to the certificate information table
 * @param   field - the field to be added (the description)
 * @param   value - the value of the field
 *
 * @details The field and value are added to the table, but only if the value has some content.
 *          If there are multiple values for the field, they are all added, but the field name is
 *          shown only once.
 */
void ConnectionDetails::addRow(QString field, QStringList value)
{
    if (value.isEmpty())
        return;

    QStringListIterator s(value);

    while(s.hasNext())
    {
        int rc = ui->certificateDetails->rowCount();
        ui->certificateDetails->insertRow(rc);
        ui->certificateDetails->setItem(rc, 0, new QTableWidgetItem(field));
        ui->certificateDetails->setItem(rc, 1, new QTableWidgetItem(s.next()));
        ui->certificateDetails->item(rc, 0)->setFont(font);

        field = "";
    }
}

/**
 * @brief   ConnectionDetails::itemClicked - shows details about a specific certificate field
 * @param   item - cell that was clicked in the table
 *
 * @details When a cell is clicked in the certificate fields table, this routine displays the details
 *          of that field in a larger area beneath the table. The idea is that some fields may not fit
 *          into the table particularly well, so this displays expanded details.
 *
 *          The field name is shown as the label above the text area, and the all values of the field are
 *          added. If the user did not click the row containing the field name, we search backwards until
 *          we find a field name that isn't blank; and then show all values until the next field name isn't
 *          blank:
 *
 *          field1     value
 *          field2     value
 *                     value
 *                     value     <-- user clicks here, but we need to show from field2 to the last value.
 *          field3     value
 */
void ConnectionDetails::itemClicked(QTableWidgetItem *item)
{
    int startRow = item->row();

    QString f = ui->certificateDetails->item(startRow, 0)->text();

    if (startRow > 0)
    {
        while(f == "")
        {
            f = ui->certificateDetails->item(--startRow, 0)->text();
        }
    }

    ui->CertificateField->setText(ui->certificateDetails->item(startRow, 0)->text());
    ui->certificateFieldDetail->setText(ui->certificateDetails->item(startRow, 1)->text());

    if (startRow < ui->certificateDetails->rowCount() - 1)
    {
        f = ui->certificateDetails->item(++startRow, 0)->text();

        while(f == "")
        {
            //ui->certificateFieldDetail->append("\n");
            ui->certificateFieldDetail->append(ui->certificateDetails->item(startRow, 1)->text());
            f = ui->certificateDetails->item(++startRow, 0)->text();
        }
    }
}

