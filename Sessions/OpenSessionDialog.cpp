/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QDebug>

#include "ActiveSettings.h"
#include "ui_SessionDialog.h"
#include "OpenSessionDialog.h"
#include "Stores/SessionStore.h"
#include "Models/Session.h"


OpenSessionDialog::OpenSessionDialog(SessionStore &store, ActiveSettings &activeSettings, QWidget *parent)
    : SessionDialogBase(store, parent), activeSettings(activeSettings)
{
    setWindowTitle("Open Session");
    setOKButtonText("Open");
    enableOKButton(false);

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &OpenSessionDialog::onOpenClicked);
//    connect(ui->sessionNameEdit, &QLineEdit::textChanged, this, &SaveSessionDialog::saveSessionNameEdited);
}

void OpenSessionDialog::onOpenClicked()
{
    Session s;

    s = store.getSession(ui->sessionNameEdit->text().trimmed());

    s.toActiveSettings(activeSettings);

    // Name and Description come from the dialog box
    // s.name = ui->sessionNameEdit->text().trimmed();
    // s.description = ui->sessionDescEdit->text().trimmed();

//    if (!store.loadSession(nameItem->text())) {
//        QMessageBox::critical(this, "Opwn Failed", "Could not open the session.");
 //       return;
 //   }

    accept(); // Close dialog with success
}
