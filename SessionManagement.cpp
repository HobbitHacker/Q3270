#include "SessionManagement.h"
#include "ui_SaveSession.h"
#include "ui_OpenSession.h"

SessionManagement::SessionManagement() : QDialog()
{

}

SessionManagement::~SessionManagement()
{

}

/*
 * Save Session Methods
 *
 */

void SessionManagement::saveSession(TerminalTab *terminal)
{
    // Build UI
    QDialog saveDialog;

    save = new Ui::SaveSession;

    save->setupUi(&saveDialog);
    saveDialog.show();

    // Signals we are interested in
    connect(save->sessionName, &QLineEdit::textChanged, this, &SessionManagement::saveSessionNameEdited);
    connect(save->tableWidget, &QTableWidget::cellClicked, this, &SessionManagement::saveRowClicked);

    // Default to OK button being disabled
    save->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // Populate supplied name into dialog box
    save->sessionName->setText("");

    // Build table
    populateTable(save->tableWidget);

    // Extract list of session names for comparison
    QStringList sessionList;
    for (int i = 0; i < save->tableWidget->rowCount(); i++)
    {
        sessionList.append(save->tableWidget->item(i, 0)->text());
    }

    // Flag to show whether user wishes to save the settings
    bool saveIt;

    // Infinite loop
    for(;;)
    {
        if (saveDialog.exec() == QDialog::Accepted)
        {
            saveIt = true;

            // If the session name already exists, prompt to overwrite, else use the name
            if (sessionList.contains(save->sessionName->text()))
            {
                QMessageBox msgBox;
                msgBox.setText("Overwrite " + save->sessionName->text() + "?");
                msgBox.setInformativeText(save->sessionName->text() + " already exists; do you want to overwrite it?");
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

                // Set flag according to whether user pressed OK
                saveIt = (msgBox.exec() == QMessageBox::Ok);
            }

            if (saveIt)
            {
                //  User either chose a unique name or confirmed overwrite
                saveSettings(terminal);
                delete save;
                return;
            }
        }
        else
        {
            // User pressed cancel
            delete save;
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

void SessionManagement::saveSettings(TerminalTab *terminal)
{

    QSettings settings;

    // Sessions are stored under the "Sessions" key, under their key of their name
    settings.beginGroup("Sessions");

    // Each session is stored under the Sessions/<session name> key
    settings.beginGroup((save->sessionName->text()));
    settings.setValue("Description", save->lineEdit->text());
    settings.setValue("ColourTheme", terminal->getColourTheme());
    settings.setValue("Address", terminal->address());

    // End group for session
    settings.endGroup();

    // End group for all sessions
    settings.endGroup();
}

/*
 * Open Session Methods
 *
 */

void SessionManagement::openSession(TerminalTab *t)
{
    // Build UI
    QDialog openDialog(0, 0);

    load = new Ui::OpenSession;

    load->setupUi(&openDialog);
    openDialog.show();

    // Signals we are interested in
    connect(load->tableWidget, &QTableWidget::cellClicked, this, &SessionManagement::openRowClicked);

    // Default to OK button being disabled
    load->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // Populate table
    populateTable(load->tableWidget);

    // Process open request if 'OK' (or double-clicked)
    if (openDialog.exec() != QDialogButtonBox::Cancel)
    {
        QSettings settings;

        // Position at Sessions group
        settings.beginGroup("Sessions");
        // Position at Session Name sub-group
        settings.beginGroup(load->tableWidget->item(load->tableWidget->currentRow(), 0)->text());
        // Set colour theme
        t->setColourTheme(settings.value("ColourTheme").toString());
    }

    delete load;
}

void SessionManagement::openRowClicked(int x, int y)
{
    load->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

/*
 * Utility Methods
 *
 */

void SessionManagement::populateTable(QTableWidget *table)
{
    // Extract current Session names and descriptions, and add to table
    QSettings settings;
    settings.beginGroup("Sessions");

    // Get a list of all existing sessions
    QStringList sessionList = settings.childGroups();

    // Empty table first
    table->setRowCount(0);

    // Populate session table
    for(int i = 0;i < sessionList.count(); i++)
    {
        // Extract session description
        settings.beginGroup(sessionList.at(i));

        QString description = settings.value("Description").toString();

        // Add session details to table
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(sessionList.at(i)));
        table->setItem(i, 1, new QTableWidgetItem(description));

        // End group for this session
        settings.endGroup();
    }

    // Fit table to window
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}
