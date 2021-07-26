#include "SessionManagement.h"
#include "ui_SaveSession.h"
#include "ui_OpenSession.h"
#include "ui_ManageSessions.h"
#include "ui_AutoStart.h"
#include "ui_AutoStartAdd.h"

/**
 * @brief SessionManagement::SessionManagement
 *
 * This class handles all aspects of Session management:
 *
 *    - Save & Load sessions
 *    - Delete session
 *    - Add/Delete to the Autostart list
 *
 * Sessions contain all custom settings
 */

SessionManagement::SessionManagement(Settings *settings) : QDialog()
{
    this->settings = settings;
}

SessionManagement::~SessionManagement()
{

}

/**
 * --------------------------------------------------------------------------------------
 * Save Session Methods
 * --------------------------------------------------------------------------------------
 */

bool SessionManagement::saveSessionAs(TerminalTab *terminal)
{
    // Build UI
    QDialog saveDialog;

    save = new Ui::SaveSession;

    save->setupUi(&saveDialog);

    // Signals we are interested in
    connect(save->sessionName, &QLineEdit::textChanged, this, &SessionManagement::saveSessionNameEdited);
    connect(save->tableWidget, &QTableWidget::cellClicked, this, &SessionManagement::saveRowClicked);

    // Default to OK button being disabled
    save->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // Clear session name
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

                // Store session name and description in case it changed
                sessionName = save->sessionName->text();
                sessionDesc = save->lineEdit->text();

                // Save settings
                saveSettings();
                delete save;
                return true;
            }
        }
        else
        {
            // User pressed cancel
            delete save;

            // Return true if this was a named session beforehand
            if (!sessionName.isNull())
            {
                return true;
            }

            // Not a named session
            return false;
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

/*!
 * \brief SessionManagement::saveSettings
 * \param terminal
 */
void SessionManagement::saveSettings()
{

    QSettings s;

    // Sessions are stored under the "Sessions" key, under their key of their name
    s.beginGroup("Sessions");

    // Each session is stored under the Sessions/<session name> key
    s.beginGroup(sessionName);
    s.setValue("Description", sessionDesc);
    s.setValue("ColourTheme", settings->getColourTheme());
    s.setValue("KeyboardTheme", settings->getKeyboardTheme());
    s.setValue("Address", settings->getAddress());
    s.setValue("TerminalModel", settings->getModel());
    s.setValue("TerminalX", settings->getTermX());
    s.setValue("TerminalY", settings->getTermY());
    s.setValue("CursorBlink", settings->getBlink());
    s.setValue("CursorBlinkSpeed", settings->getBlinkSpeed());
    s.setValue("CursorInheritColour", settings->getInherit());
    s.setValue("Ruler", settings->getRulerOn());
    s.setValue("RulerStyle", settings->getRulerStyle());
    s.setValue("Font", settings->getFont().family());
    s.setValue("FontSize", settings->getFont().pointSize());
    s.setValue("FontStyle", settings->getFontScaling());
    s.setValue("FontScaling", settings->getFontScaling());
    s.setValue("ScreenStretch", settings->getStretch());

    // End group for session
    s.endGroup();

    // End group for all sessions
    s.endGroup();

    //TODO: Valid characters in session name

    // Store session name
    sessionName = save->sessionName->text();
}

/**
 * --------------------------------------------------------------------------------------
 * Open Session Methods
 * --------------------------------------------------------------------------------------
 */

bool SessionManagement::openSession(TerminalTab *t)
{
    // Build UI
    QDialog openDialog;

    load = new Ui::OpenSession;

    load->setupUi(&openDialog);

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

        // Open named session
        openSession(t, load->tableWidget->item(load->tableWidget->currentRow(), 0)->text());

        // Save session name and description
        sessionName = load->tableWidget->item(load->tableWidget->currentRow(), 0)->text();
        sessionDesc = load->tableWidget->item(load->tableWidget->currentRow(), 1)->text();

        delete load;

        return true;
    }

    delete load;

    // If this was a named session beforehand, return true
    if (!sessionName.isNull())
    {
        return true;
    }

    // Not a named session
    return false;
}

void SessionManagement::openSession(TerminalTab *t, QString sessionName)
{
    QSettings s;

    // Position at Sessions group
    s.beginGroup("Sessions");

    // Position at Session Name sub-group
    s.beginGroup(sessionName);

    if (!s.childKeys().isEmpty())
    { 
        // Set themes
        t->setColourTheme(s.value("ColourTheme").toString());
        t->setKeyboardTheme(s.value("KeyboardTheme").toString());

        // Set terminal characteristics
        settings->setTerminalModel(s.value("TerminalModel").toString());
        settings->setTerminalSize(s.value("TerminalX").toInt(), s.value("TerminalY").toInt());

        // Cursor settings
        settings->setBlink(s.value("CursorBlink").toBool());
        settings->setBlinkSpeed(s.value("CursorBlinkSpeed").toInt());
        settings->setInherit(s.value("CursorInheritColour").toBool());

        // Ruler
        settings->setRulerOn(s.value("Ruler").toBool());
        settings->setRulerStyle(s.value("RulerStyle").value<DisplayScreen::RulerStyle>());

        // Font settings
        QFont f;
        f.setFamily(s.value("Font").toString());
        f.setPointSize(s.value("FontSize").toInt());
        f.setStyleName(s.value("FontStyle").toString());

        settings->setFont(f);

        settings->setFontScaling(s.value("FontScaling").toBool());
        settings->setStretch(s.value("ScreenStretch").toBool());

        t->openConnection(s.value("Address").toString());
        t->setSessionName(sessionName);

        // Update MRU entries
        emit sessionOpened("Session " + sessionName);

    }

    s.endGroup();
    s.endGroup();
}

void SessionManagement::openRowClicked(int x, int y)
{
    load->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}


/**
 * --------------------------------------------------------------------------------------
 * Manage Session Methods
 * --------------------------------------------------------------------------------------
 */

void SessionManagement::manageSessions()
{
    // Dialog for sessions management
    QDialog m;

    // Build UI
    manage = new Ui::ManageSessions;

    manage->setupUi(&m);

    // Populate UI table with session details
    populateTable(manage->sessionList);

    // Signals we are interested in
    connect(manage->deleteSession, &QPushButton::clicked, this, &SessionManagement::deleteSession);
    connect(manage->sessionList, &QTableWidget::cellClicked, this, &SessionManagement::manageRowClicked);
    connect(manage->buttonManageAutoStart, &QPushButton::clicked, this, &SessionManagement::manageAutoStartList);

    if (m.exec() == QDialog::Accepted)
    {
        // TODO: Save sessions when OK pressed; for now, Delete is actioned at point of
        // pressing the Delete button
    }

    delete manage;
}

void SessionManagement::deleteSession()
{

    QSettings settings;

    // Narrow to Sessions group
    settings.beginGroup("Sessions");

    // Remove selected item
    settings.remove(manage->sessionList->item(manage->sessionList->currentRow(), 0)->text());

    // Clear group filter
    settings.endGroup();

    // Re-populate table
    populateTable(manage->sessionList);

    // Reset delete button
    manage->deleteSession->setDisabled(true);

}

void SessionManagement::manageRowClicked(int x, int y)
{
    // Enable the Delete Session button when a row is clicked
    manage->deleteSession->setEnabled(true);
}

/**
 * --------------------------------------------------------------------------------------
 * Manage Autostart Session Methods
 * --------------------------------------------------------------------------------------
 */

void SessionManagement::manageAutoStartList()
{
    QSettings settings;

    // Used to access the session details
    QSettings sessionSettings;

    // Focus on the sessions
    sessionSettings.beginGroup("Sessions");

    // Dialog for autostart list management
    QDialog a;

    // Build UI
    autostart = new Ui::AutoStart;

    autostart->setupUi(&a);

    // Read the sessions in the autostart list
    int ac = settings.beginReadArray("AutoStart");

    // Clear the dialog table
    autostart->sessionList->setRowCount(0);

    for (int i = 0;i < ac; i++)
    {
        // Get the session name from the AutoStart array in the config file
        settings.setArrayIndex(i);
        QString thisEntry = settings.value("Session").toString();

        // Pick up the description from the Session's details in the config file
        sessionSettings.beginGroup(thisEntry);
        QString thisDesc = sessionSettings.value("Description").toString();
        sessionSettings.endGroup();

        // Populate the dialog table
        autostart->sessionList->insertRow(i);
        autostart->sessionList->setItem(i, 0, new QTableWidgetItem(thisEntry));
        autostart->sessionList->setItem(i, 1, new QTableWidgetItem(thisDesc));

    }

    // Finish AutoStart group
    settings.endArray();

    // Make table fit across dialog
    autostart->sessionList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Add, Delete and Row Select button signals
    connect(autostart->sessionList, &QTableWidget::cellClicked, this, &SessionManagement::autoStartCellClicked);
    connect(autostart->addButton, &QPushButton::clicked, this, &SessionManagement::addAutoStart);
    connect(autostart->deleteButton, &QPushButton::clicked, this, &SessionManagement::deleteAutoStart);

    // Signal when a new row is added to the autostart list
    connect(this, &SessionManagement::autoStartAddToList, this, &SessionManagement::autoStartRowAdded);

    // Process dialog
    if (a.exec() == QDialog::Accepted)
    {
        // Clear the autostart array
        settings.remove("AutoStart");

        // Create new autostart list
        settings.beginWriteArray("AutoStart");

        for (int i = 0;i < autostart->sessionList->rowCount(); i++)
        {
            // Insert the value from the table (note description not stored)
            settings.setArrayIndex(i);
            settings.setValue("Session", autostart->sessionList->item(i, 0)->text());
        }

        // Finish AutoStart group
        settings.endArray();
    }

    // Tidy up
    delete autostart;
}

void SessionManagement::autoStartRowAdded(int row)
{
    // Insert a new row to Autostart table
    autostart->sessionList->insertRow(autostart->sessionList->rowCount());

    // Create copies of table items (items cannot be owned by multiple tables)
    QTableWidgetItem *as1 = add->sessionList->item(add->sessionList->currentRow(), 0)->clone();
    QTableWidgetItem *as2 = add->sessionList->item(add->sessionList->currentRow(), 1)->clone();

    // Add details to row just added (rowCount - 1 is new row, zero based)
    autostart->sessionList->setItem(autostart->sessionList->rowCount() - 1, 0, as1);
    autostart->sessionList->setItem(autostart->sessionList->rowCount() - 1, 1, as2);
}

void SessionManagement::autoStartCellClicked(int row, int col)
{
    // Enable Delete button when a cell is selected
    autostart->deleteButton->setEnabled(true);
}

void SessionManagement::addAutoStart()
{

    // Build dialog for adding a new autostart session to the list.
    QDialog a;

    add = new Ui::AutoStartAdd;

    add->setupUi(&a);

    // Default to OK button being disabled
    add->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // Populate the table
    populateTable(add->sessionList);

    //  Enable OK button if a cell is selected
    connect(add->sessionList, &QTableWidget::cellClicked, this, &SessionManagement::autoStartAddCellClicked);

    if (a.exec() == QDialog::Accepted)
    {
        emit autoStartAddToList(add->sessionList->currentRow());
    }

    delete add;
}

void SessionManagement::autoStartAddCellClicked(int x, int y)
{
    // Enable OK button
    add->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void SessionManagement::deleteAutoStart()
{
    // Remove currently selected row
    autostart->sessionList->removeRow(autostart->sessionList->currentRow());

    // Disable delete button again, now selected row is gone
    autostart->deleteButton->setDisabled(true);
}


/*
 * Utility Methods
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

        // Add session details to table
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(sessionList.at(i)));
        table->setItem(i, 1, new QTableWidgetItem(settings.value("Description").toString()));
        table->setItem(i, 2, new QTableWidgetItem(settings.value("Address").toString()));

        // End group for this session
        settings.endGroup();
    }

    // Fit table to window
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}
