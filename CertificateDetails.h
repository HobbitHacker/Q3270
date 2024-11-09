#ifndef CERTIFICATEDETAILS_H
#define CERTIFICATEDETAILS_H

#include <QDialog>
#include <QSslCertificate>
#include <QTableWidgetItem>

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

        void addRow(QString field, QStringList value);

        QFont font;

    private slots:
        void showCertificate(int i);
        void itemClicked(QTableWidgetItem *t);
};

#endif // CERTIFICATEDETAILS_H
