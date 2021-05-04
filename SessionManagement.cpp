#include "SessionManagement.h"
#include "ui_SaveSession.h"

SessionManagement::SessionManagement(TerminalTab *t) : QDialog()
{

}

SessionManagement::~SessionManagement()
{

}

void SessionManagement::saveSession()
{
    // Build UI
    QDialog *saveDialog = new QDialog(0, 0);

    save = new Ui::SaveSession;

    save->setupUi(saveDialog);
    saveDialog->show();

    // Ensure table stretches to full width
    save->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Signals we are interested in
    connect(save->sessionName, &QLineEdit::textChanged, this, &SessionManagement::saveSessionNameEdited);
    connect(save->tableWidget, &QTableWidget::cellClicked, this, &SessionManagement::saveRowClicked);

    // Default to OK button being disabled
    save->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // Populate supplied name into dialog box
    save->sessionName->setText("");

    // Extract current Session names and descriptions, and add to table
    QSettings settings;
    settings.beginGroup("Sessions");

    // Get a list of all existing sessions
    QStringList sessionList = settings.childGroups();

    // Empty table first
    save->tableWidget->setRowCount(0);

    // Populate session table
    for(int i = 0;i < sessionList.count(); i++)
    {
        // Extract session description
        settings.beginGroup(sessionList.at(i));

        QString description = settings.value("Description").toString();

        // Add session details to table
        save->tableWidget->insertRow(i);
        save->tableWidget->setItem(i, 0, new QTableWidgetItem(sessionList.at(i)));
        save->tableWidget->setItem(i, 1, new QTableWidgetItem(description));

        // End group for this session
        settings.endGroup();
    }

    // Fit table to window
    save->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Infinite loop
    for(;;)
    {
        if (saveDialog->exec())
        {
            // If the session name already exists, prompt to overwrite, else use the name
            if (sessionList.contains(save->sessionName->text()))
            {
                QMessageBox msgBox;
                msgBox.setText("Overwrite " + save->sessionName->text() + "?");
                msgBox.setInformativeText(save->sessionName->text() + " already exists; do you want to overwrite it?");
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                // User has pressed OK, so return the details
                if (msgBox.exec() == QMessageBox::Ok)
                {
                    saveSettings();
                    return;
                }
            }
            else
            {
                // It's a unique name, return it
                saveSettings();
                return;
            }
        }
        else
        {
            // User pressed cancel
            return;
        }
    }
}

void SessionManagement::saveSessionNameEdited(QString name)
{
    // If the Session Name is empty, disable the OK button
    if (name.isEmpty())
    {
        save->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    }
    else
    {
        save->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}

void SessionManagement::saveRowClicked(int row, int column)
{
    // Populate text fields from table cells
    save->sessionName->setText(save->tableWidget->item(row, 0)->text());
    save->lineEdit->setText(save->tableWidget->item(row, 1)->text());
}

void SessionManagement::saveSettings()
{

    QSettings settings;

    // Sessions are stored under the "Sessions" key, under their key of their name
    settings.beginGroup("Sessions");

    // Each session is stored under the Sessions/<session name> key
    settings.beginGroup((save->sessionName->text()));
    settings.setValue("Description", save->lineEdit->text());
    settings.setValue("ColourTheme", "Factory");

    // End group for session
    settings.endGroup();

    // End group for all sessions
    settings.endGroup();
}
