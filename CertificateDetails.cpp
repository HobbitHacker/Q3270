/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <QDebug>

#include "CertificateDetails.h"
#include "ui_CertificateDetails.h"

/**
 *  @brief   CertificateDetails::CertificateDetails - Display Certificate Details
 *
 *  @details This class displays details about certificates used for the connection. Certificates are
 *           extracted from the connection and stored in this class because each certificate is displayed
 *           individually
 */

CertificateDetails::CertificateDetails(const QList<QSslCertificate> &certlist, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateDetails),
    certs(certlist)

{
    ui->setupUi(this);

    connect(ui->certificateList, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CertificateDetails::showCertificate);

    ui->certificateList->clear();

    for (int i = 0; i < certlist.size(); ++i)
    {
        const QSslCertificate &cert = certlist.at(i);
        ui->certificateList->addItem (tr("%1%2 (%3)").arg(!i ? tr("This site: ") : tr("Issued by: "))
                                             .arg(cert.subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' ')))
                                             .arg(cert.subjectInfo(QSslCertificate::CommonName).join(QLatin1Char(' '))));
    }

    ui->certificateList->setCurrentIndex(0);

    font.setBold(true);
}

/**
 * @brief   CertificateDetails::~CertificateDetails - destructor
 *
 * @details Delete the ui object created in the constructor.
 */
CertificateDetails::~CertificateDetails()
{
    delete ui;
}

/**
 * @brief   CertificateDetails::showCertificate - show a specific certificate
 * @param   i - the index of the certificate in the drop-down list
 *
 * @details This is called when the user selects a different certificate from the drop-down list.
 *          The fields are added to the table in the dialog, and are only shown if they have content.
 *          (ie, blank fields are ignored).
 */
void CertificateDetails::showCertificate(int i)
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
//        addRow(tr("MD5 Digest"), cert.digest().to));
    }

    ui->certificateDetails->resizeRowsToContents();
}

/**
 * @brief   CertificateDetails::addRow - add a row to the certificate information table
 * @param   field - the field to be added (the description)
 * @param   value - the value of the field
 *
 * @details The field and value are added to the table, but only if the value has some content.
 *          If there are multiple values for the field, they are all added, but the field name is
 *          shown only once.
 */
void CertificateDetails::addRow(QString field, QStringList value)
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
 * @brief   CertificateDetails::itemClicked - shows details about a specific certificate field
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
void CertificateDetails::itemClicked(QTableWidgetItem *item)
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

