/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef CONNECTIONDETAILS_H
#define CONNECTIONDETAILS_H

#include <QDialog>
#include <QSslCertificate>
#include <QTableWidgetItem>

#include "Terminal.h"
#include "ActiveSettings.h"

namespace Ui {
    class ConnectionDetails;
}

class ConnectionDetails : public QDialog
{
    Q_OBJECT

    public:
        explicit ConnectionDetails(Terminal *terminal, ActiveSettings &activeSettings, QWidget *parent = nullptr);
        ~ConnectionDetails();

    private:
        Terminal *terminal;
        ActiveSettings &activeSettings;
        Ui::ConnectionDetails *ui;

        QList<QSslCertificate> certs;

        void addRow(QString field, QStringList value);

        QFont font;

    private slots:
        void showCertificate(int i);
        void itemClicked(QTableWidgetItem *t);
};

#endif // CONNECTIONDETAILS_H
