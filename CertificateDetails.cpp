#include <QDebug>

#include "CertificateDetails.h"
#include "ui_CertificateDetails.h"

CertificateDetails::CertificateDetails(const QList<QSslCertificate> &certlist, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CertificateDetails),
    certs(certlist)

{
    ui->setupUi(this);

    connect(ui->certificateList, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CertificateDetails::showCertificate);

    ui->certificateList->clear();
    for (int i = 0; i < certlist.size(); ++i) {
        const QSslCertificate &cert = certlist.at(i);
        ui->certificateList->addItem (tr("%1%2 (%3)").arg(!i ? tr("This site: ") : tr("Issued by: "))
                                             .arg(cert.subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' ')))
                                             .arg(cert.subjectInfo(QSslCertificate::CommonName).join(QLatin1Char(' '))));
        qDebug() << cert.subjectDisplayName();
        qDebug() << cert.subjectInfo(QSslCertificate::CommonName);
        qDebug() << cert.effectiveDate();
        qDebug() << cert.expiryDate();

    }

    ui->certificateList->setCurrentIndex(0);
}

CertificateDetails::~CertificateDetails()
{
    delete ui;
}

void CertificateDetails::showCertificate(int i)
{
    ui->certificateDetail->clear();
    if (i >= 0 && i < certs.size()) {
        const QSslCertificate &cert = certs.at(i);
        QStringList lines;
        lines << tr("Organization: %1").arg(cert.subjectInfo(QSslCertificate::Organization).join(u' '))
              << tr("Subunit: %1").arg(cert.subjectInfo(QSslCertificate::OrganizationalUnitName).join(u' '))
              << tr("Country: %1").arg(cert.subjectInfo(QSslCertificate::CountryName).join(u' '))
              << tr("Locality: %1").arg(cert.subjectInfo(QSslCertificate::LocalityName).join(u' '))
              << tr("State/Province: %1").arg(cert.subjectInfo(QSslCertificate::StateOrProvinceName).join(u' '))
              << tr("Common Name: %1").arg(cert.subjectInfo(QSslCertificate::CommonName).join(u' '))
              << tr("Subject Alternative Names");

         for (const auto &san : cert.subjectAlternativeNames())
                 lines << san;

          lines << QString()
              << tr("Issuer Organization: %1").arg(cert.issuerInfo(QSslCertificate::Organization).join(u' '))
              << tr("Issuer Unit Name: %1").arg(cert.issuerInfo(QSslCertificate::OrganizationalUnitName).join(u' '))
              << tr("Issuer Country: %1").arg(cert.issuerInfo(QSslCertificate::CountryName).join(u' '))
              << tr("Issuer Locality: %1").arg(cert.issuerInfo(QSslCertificate::LocalityName).join(u' '))
              << tr("Issuer State/Province: %1").arg(cert.issuerInfo(QSslCertificate::StateOrProvinceName).join(u' '))
              << tr("Issuer Common Name: %1").arg(cert.issuerInfo(QSslCertificate::CommonName).join(u' '));
        for (const auto &line : lines)
            ui->certificateDetail->addItem(line);
}
}
