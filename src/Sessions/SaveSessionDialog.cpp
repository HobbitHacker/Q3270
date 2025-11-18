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

#include "ActiveSettings.h"
#include "ui_SessionDialog.h"
#include "SaveSessionDialog.h"


/**
 * @brief   SaveSessionDialog::SaveSessionDialog constructor.
 * @param   store               Reference to the SessionStore.
 * @param   activeSettings      Reference to the ActiveSettings.
 * @param   parent              Parent widget.
 * 
 * @details This dialog allows the user to save the current active settings as a session.
 */
SaveSessionDialog::SaveSessionDialog(SessionStore &store, ActiveSettings &activeSettings, QWidget *parent)
    : SessionDialogBase(store, parent), activeSettings(activeSettings)
{
    setWindowTitle("Save Session");
    setOKButtonText("Save");

    // Delete not available from 'Save Session as'
    ui->deleteButton->setVisible(false);

    ui->sessionNameEdit->setText(activeSettings.getSessionName());
    ui->sessionDescEdit->setText(activeSettings.getDescription());

    ui->previewWidget->setSession(Session::fromActiveSettings(activeSettings));

    connect(ui->sessionNameEdit, &QLineEdit::textChanged, this, &SaveSessionDialog::saveSessionNameEdited);

    ui->sessionNameEdit->setReadOnly(false);
    ui->sessionDescEdit->setReadOnly(false);
}

/**
  * @brief   SaveSessionDialog::onAccept Slot called when the Save button is clicked.
  * 
  * @details This function saves the current active settings as a new session
  *          with the name and description provided in the dialog. If a session
  *          with the same name already exists, the user is prompted to confirm
  *          overwriting it.
  */
void SaveSessionDialog::onAccept()
{
    Session s = Session::fromActiveSettings(activeSettings);

    // Name and Description come from the dialog box
    s.name = ui->sessionNameEdit->text().trimmed();
    s.description = ui->sessionDescEdit->text().trimmed();

    // should be covered by the checks on editing the name
    if (s.name.isEmpty()) {
        QMessageBox::warning(this, "Missing Name", "Please enter a session name.");
        return;
    }

    QStringList existingNames = store.listSessionNames();
    if (existingNames.contains(s.name)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Confirm Overwrite");
        msgBox.setText(QString("Session '%1' already exists. Overwrite?").arg(s.name));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);

        if (msgBox.exec() != QMessageBox::Ok)
            return;
    }

    if (!store.saveSession(s)) {
        QMessageBox::critical(this, "Save Failed", "Could not save the session.");
        return;
    }

    activeSettings.setSessionName(s.name);
    activeSettings.setDescription(s.description);

    accept(); // Close dialog with success
}

/**
 * @brief   SaveSessionDialog::saveSessionNameEdited Slot called when the session name is edited.
 * @param   name        The new session name.
 * 
 * @details This function enables or disables the OK button based on whether
 *          the session name is empty.
 */
void SaveSessionDialog::saveSessionNameEdited(const QString &name)
{
    bool hasName = name.trimmed().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!hasName);
}
