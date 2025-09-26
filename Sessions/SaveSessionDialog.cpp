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

#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>

#include "ActiveSettings.h"
#include "ui_SessionDialog.h"
#include "SaveSessionDialog.h"


SaveSessionDialog::SaveSessionDialog(SessionStore &store, ActiveSettings &activeSettings, QWidget *parent)
    : SessionDialogBase(store, parent), activeSettings(activeSettings)
{
    setWindowTitle("Save Session");
    setOKButtonText("Save");

    ui->sessionNameEdit->setText(activeSettings.getSessionName());
    ui->sessionDescEdit->setText(activeSettings.getDescription());

    ui->previewWidget->setSession(Session::fromActiveSettings(activeSettings));

    //enableOKButton(false);

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &SaveSessionDialog::onSaveClicked);
    connect(ui->sessionNameEdit, &QLineEdit::textChanged, this, &SaveSessionDialog::saveSessionNameEdited);

    ui->sessionNameEdit->setReadOnly(false);
    ui->sessionDescEdit->setReadOnly(false);
}

void SaveSessionDialog::onSaveClicked()
{
    Session s = Session::fromActiveSettings(activeSettings);

    // Name and Description come from the dialog box
    s.name = ui->sessionNameEdit->text().trimmed();
    s.description = ui->sessionDescEdit->text().trimmed();

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

void SaveSessionDialog::saveSessionNameEdited(const QString &name)
{
    bool hasName = name.trimmed().isEmpty();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!hasName);
}
