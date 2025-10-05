/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QPushButton>
#include <QDialogButtonBox>

#include "ManageSessionsDialog.h"
#include "ManageAutoStartDialog.h"
#include "Stores/SessionStore.h"
//#include "ManageAutoStartDialog.h"
#include "ui_SessionDialog.h"

ManageSessionsDialog::ManageSessionsDialog(SessionStore &store, QWidget *parent)
    : SessionDialogBase(store, parent)
{
    setWindowTitle(tr("Manage Sessions"));

    // Add the "Manage AutoStart..." button alongside the base class buttons
    autoStartButton = new QPushButton(tr("Manage AutoStart..."), this);
    ui->buttonBox->addButton(autoStartButton, QDialogButtonBox::ActionRole);

    connect(autoStartButton, &QPushButton::clicked, this, &ManageSessionsDialog::onManageAutoStartClicked);
    connect(this, &SessionDialogBase::deleteRequested, this,&ManageSessionsDialog::doDelete);
}

void ManageSessionsDialog::onManageAutoStartClicked()
{
    ManageAutoStartDialog dlg(store, this);
    dlg.exec();
}
