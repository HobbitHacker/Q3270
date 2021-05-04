#include "SaveSession.h"
#include "ui_SaveSession.h"

SaveSession::SaveSession(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveSession)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->sessionName, &QLineEdit::textEdited, this, &SaveSession::sessionNameEdited);

    // Default to OK button being disabled
    ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

}

QString SaveSession::getSessionName(QString defaultName)
{
    // Set up a new instance
    SaveSession *save = new SaveSession();

    // Populate supplied name into dialog box
    save->ui->sessionName->setText(defaultName);

    // Call the edit slot to adjust OK button
    save->sessionNameEdited(defaultName);

    // Extract current Session names and descriptions, and add to table
    QSettings settings;
    settings.beginGroup("Sessions");

    // Get a list of all existing sessions
    QStringList sessionList = settings.childGroups();

    // Empty table first
    save->ui->tableWidget->setRowCount(0);

    // Populate session table
    for(int i = 0;i < sessionList.count(); i++)
    {
        // Extract session description
        settings.beginGroup(sessionList.at(i));

        QString description = settings.value("Description").toString();

        // Add session details to table
        save->ui->tableWidget->insertRow(i);
        save->ui->tableWidget->setItem(i, 0, new QTableWidgetItem(sessionList.at(i)));
        save->ui->tableWidget->setItem(i, 1, new QTableWidgetItem(description));
    }

    // Fit table to window
    save->ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Infinite loop
    for(;;)
    {
        if (save->exec())
        {
            // If the session name already exists, prompt to overwrite, else use the name
            if (sessionList.contains(save->ui->sessionName->text()))
            {
                QMessageBox msgBox;
                msgBox.setText("Overwrite " + save->ui->sessionName->text() + "?");
                msgBox.setInformativeText(save->ui->sessionName->text() + " already exists; do you want to overwrite it?");
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                if (msgBox.exec() == QMessageBox::Ok)
                {
                    return save->ui->sessionName->text();
                }
            }
            else
            {
                // It's a unique name, return it
                return save->ui->sessionName->text();
            }
        }
        else
        {
            // User pressed cancel
            return "";
        }
    }
}

SaveSession::~SaveSession()
{
    delete ui;
}

void SaveSession::sessionNameEdited(QString name)
{
    // If the Session Name is empty, disable the OK button
    if (name.isEmpty())
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    }
    else
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}
