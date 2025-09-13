#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>

#include "ActiveSettings.h"
#include "ui_SessionDialog.h"
#include "SaveSessionDialog.h"


SaveSessionDialog::SaveSessionDialog(ActiveSettings &activeSettings, QWidget *parent)
    : SessionDialogBase(parent), activeSettings(activeSettings)
{
    setWindowTitle("Save Session");
    setOKButtonText("Save");
    enableOKButton(false);

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &SaveSessionDialog::onSaveClicked);
    connect(ui->sessionNameEdit, &QLineEdit::textChanged, this, &SaveSessionDialog::saveSessionNameEdited);
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
