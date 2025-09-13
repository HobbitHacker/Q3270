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

#include "Q3270.h"
#include "SessionStore.h"

#include "SessionManagement.h"
#include "SaveSessionDialog.h"
#include "OpenSessionDialog.h"
#include "ui_SaveSession.h"
#include "ui_OpenSession.h"
#include "ui_ManageSessions.h"
#include "ui_AutoStart.h"
#include "ui_AutoStartAdd.h"
#include "ManageSessionsDialog.h"

/**
 * @brief   SessionManagement::SessionManagement - All aspects of Session management
 *
 * @details This handles all aspected of Session Management. Saved Sessions contain all settings that
 *          define a given session. This class allows:
 *
 *          - Save & Load sessions
 *          - Delete session
 *          - Add/Delete to the Autostart list
 */
SessionManagement::SessionManagement(ActiveSettings &activeSettings) :
    QDialog() ,
    activeSettings(activeSettings)
{

}

/**
 * @brief   SessionManagement::~SessionManagement - destructor
 *
 * @details Delete any objects obtained via 'new'
 */
SessionManagement::~SessionManagement()
{

}

/**
 * --------------------------------------------------------------------------------------
 * Save Session Methods
 * --------------------------------------------------------------------------------------
 */

/**
 * @brief   SessionManagement::saveSessionAs - save a session with a new name
 * @return  true for OK, false for Cancel
 */
bool SessionManagement::saveSessionAs()
{
    SaveSessionDialog dlg(activeSettings, this);

    if (dlg.exec() == QDialog::Accepted) {
        // Optionally refresh session list or preview
        return true;
    }


    // If the session was previously named, return true to preserve state
    return !sessionName.isNull();
}

/**
 * --------------------------------------------------------------------------------------
 * Open Session Methods
 * --------------------------------------------------------------------------------------
 */

/**
 * @brief   SessionManagement::openSession - open an existing session
 * @param   t - the terminal
 * @return  true for OK, false for Cancel
 *
 * @details Display a list of sessions for the user to select one to open a saved session. If they
 *          select one, open and connect to it.
 */

bool SessionManagement::openSession()
{
    OpenSessionDialog dlg(activeSettings, this);

    if (dlg.exec() == QDialog::Accepted)
    {
        // Update MRU entries
        emit sessionOpened();
        return true;
    }

    return false;
}


/**
 * --------------------------------------------------------------------------------------
 * Manage Session Methods
 * --------------------------------------------------------------------------------------
 */

/**
 * @brief   SessionManagement::manageSessions - show the Manage Sessions dialog
 *
 * @details Display the dialog allowing the user to delete sessions and to manage the autostart
 *          list.
 */
void SessionManagement::manageSessions()
{
    ManageSessionsDialog dlg(activeSettings, this);
    dlg.exec();
}

/**
 * --------------------------------------------------------------------------------------
 * Manage Autostart Session Methods
 * --------------------------------------------------------------------------------------
 */

/**
 * @brief   SessionManagement::manageAutoStartList - Display the autostart list of sessions
 *
 * @details The autostart list of sessions are ones that are started when Q3270 starts up. This
 *          dialog allows the user to specify which sessions are started.
 */
void SessionManagement::manageAutoStartList()
{
    QSettings settings(Q3270_ORG, Q3270_APP);

    // Used to access the session details
    QSettings sessionSettings(Q3270_ORG, Q3270_APP);

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
    autostart->sessionList->clearContents();

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

/**
 * @brief   SessionManagement::autoStartRowAdded - add a new row to the autostart list
 * @param   row - the row that was added
 *
 * @details The autostart list has a new row added, so populate this table with a copy from the
 *          list of sessions.
 */
void SessionManagement::autoStartRowAdded([[maybe_unused]] int row)
{
    // Insert a new row to Autostart table
    qDebug() << autostart->sessionList->rowCount();

    autostart->sessionList->insertRow(autostart->sessionList->rowCount());

    // Create copies of table items (items cannot be owned by multiple tables)
    QTableWidgetItem *as1 = add->sessionList->item(add->sessionList->currentRow(), 0)->clone();
    QTableWidgetItem *as2 = add->sessionList->item(add->sessionList->currentRow(), 1)->clone();

    // Add details to row just added (rowCount - 1 is new row, zero based)
    autostart->sessionList->setItem(autostart->sessionList->rowCount() - 1, 0, as1);
    autostart->sessionList->setItem(autostart->sessionList->rowCount() - 1, 1, as2);
}

/**
 * @brief   SessionManagement::autoStartCellClicked - enable the Delete button
 * @param   row - unused
 * @param   col - unused
 *
 * @details When a row is clicked in the autostart list, enable the 'Delete' button
 */
void SessionManagement::autoStartCellClicked([[maybe_unused]] int row, [[maybe_unused]] int col)
{
    // Enable Delete button when a cell is selected
    autostart->deleteButton->setEnabled(true);
}

/**
 * @brief   SessionManagement::addAutoStart - display the Add to Autostart dialog
 *
 * @details The Add to Autostart list shows a list of sessions that the user can select from to be
 *          started at Q3270 startup.
 */
void SessionManagement::addAutoStart()
{
    // Build dialog for adding a new autostart session to the list.
    QDialog a;

    add = new Ui::AutoStartAdd;

    add->setupUi(&a);

    // Default to OK button being disabled
    add->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

    // Populate the table
    //populateTable(add->sessionList);

    //  Enable OK button if a cell is selected
    connect(add->sessionList, &QTableWidget::cellClicked, this, &SessionManagement::autoStartAddCellClicked);

    if (a.exec() == QDialog::Accepted)
    {
        emit autoStartAddToList(add->sessionList->currentRow());
    }

    delete add;
}

/**
 * @brief   SessionManagement::autoStartAddCellClicked - enable the Add button
 * @param   x - unused
 * @param   y - unused
 *
 * @details When the user clicks on a row in the table, enable the Add button.
 */
void SessionManagement::autoStartAddCellClicked([[maybe_unused]] int x, [[maybe_unused]] int y)
{
    // Enable OK button
    add->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

/**
 * @brief   SessionManagement::deleteAutoStart - delete a row from the autostart list
 *
 * @details Remove the selected row from the autostart list and disable the Delete button.
 */
void SessionManagement::deleteAutoStart()
{
    // Remove currently selected row
    autostart->sessionList->removeRow(autostart->sessionList->currentRow());

    // Disable delete button again, now selected row is gone
    autostart->deleteButton->setDisabled(true);
}
