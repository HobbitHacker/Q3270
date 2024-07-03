#ifndef CERTIFICATEDETAILS_H
#define CERTIFICATEDETAILS_H

#include <QDialog>
#include <QSslCertificate>

namespace Ui {
    class CertificateDetails;
}

class CertificateDetails : public QDialog
{
    Q_OBJECT

    public:
        explicit CertificateDetails(const QList<QSslCertificate> &list, QWidget *parent = nullptr);
        ~CertificateDetails();

    private:
        Ui::CertificateDetails *ui;

        QList<QSslCertificate> certs;

    private slots:
        void showCertificate(int i);


};

#endif // CERTIFICATEDETAILS_H
