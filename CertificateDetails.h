/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

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
