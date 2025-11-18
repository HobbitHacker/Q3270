/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include "ui_SessionDialog.h"
#include "ui_SessionPreview.h"
#include "SessionDialogBase.h"
#include "HostAddressUtils.h"

/**
 * @brief   SessionDialogBase constructor.
 * @param   store               Reference to the SessionStore.
 * @param   parent              Parent widget.
 *
 * @details This is the base class for session dialogs.
 */
SessionDialogBase::SessionDialogBase(SessionStore &store, QWidget *parent)
    : QDialog(parent), ui(new Ui::SessionDialog), store(store)
{
    ui->setupUi(this);

    sessions = store.listSessions();

    setupTable();
    connectSignals();

    populateSessionTable();

    ui->sessionTable->clearSelection();
    ui->sessionTable->setCurrentIndex(QModelIndex());
}

/**
 * @brief   SessionDialogBase destructor.
 * 
 * @details Cleans up the UI.
 * @note    Probably not needed since ui is a pointer to a QObject-derived class,
 *          which will be automatically deleted when the parent QDialog is deleted.
 */
SessionDialogBase::~SessionDialogBase()
{
    delete ui;
}

/**
 * @brief  setupTable Sets up the session table widget.
 * 
 * @details Configures the session table with two columns: "Session Name" and "Description".
 *          It also sets the selection behavior to select entire rows and disables editing. 
 */
void SessionDialogBase::setupTable()
{
    ui->sessionTable->setColumnCount(2);
    ui->sessionTable->setHorizontalHeaderLabels({"Session Name", "Description"});
    ui->sessionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->sessionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->sessionTable->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->sessionTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->sessionTable->horizontalHeader()->setStretchLastSection(true); // Or use setSectionResizeMode in code

    // Reduce vertical padding (tweak +4 to adjust)
    ui->sessionTable->verticalHeader()->setDefaultSectionSize(ui->sessionTable->fontMetrics().height() + 4);

    // Optional: disable word wrap so rows don't expand unexpectedly
    ui->sessionTable->setWordWrap(false);

}

/**
 * @brief   populateSessionTable Populates the session table with sessions from the store.
 * 
 * @details This function retrieves the list of sessions from the SessionStore
 *          and populates the session table with their names and descriptions.
 */
void SessionDialogBase::populateSessionTable()
{
    sessions = store.listSessions();

    ui->sessionTable->setRowCount(sessions.size());

    for (int i = 0; i < sessions.size(); ++i) {
        const Session &s = sessions[i];
        ui->sessionTable->setItem(i, 0, new QTableWidgetItem(s.name));
        ui->sessionTable->setItem(i, 1, new QTableWidgetItem(s.description));
    }
}

/**
 * @brief   connectSignals Connects UI signals to their respective slots.
 * 
 * @details This function connects the dialog's button box signals (accepted and rejected)
 *          to the appropriate slots, as well as the delete button and session table cell click events.
 */
void SessionDialogBase::connectSignals()
{
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SessionDialogBase::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SessionDialogBase::reject);
    connect(ui->deleteButton, &QPushButton::clicked, this, &SessionDialogBase::requestDeleteSelected);
    connect(ui->sessionTable, &QTableWidget::cellClicked, this, &SessionDialogBase::onRowClicked);
}

/**
 * @brief   enableOKButton Enables or disables the OK button.
 * @param   enabled     True to enable, false to disable.
 * 
 * @details This function enables or disables the OK button in the dialog's button box.
 */
void SessionDialogBase::enableOKButton(bool enabled)
{
    if (ui->buttonBox)
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

/**
 * @brief   setOKButtonText Sets the text of the OK button.
 * @param   text        The text to set on the OK button.
 * 
 * @details This function sets the text of the OK button in the dialog's button box.
 */
void SessionDialogBase::setOKButtonText(const QString &text)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(text);
}

/**
 * @brief   onAccept Slot called when the OK button is clicked.
 * 
 * @details This function is called when the OK button is clicked.
 *          Subclasses can override this method to implement custom behavior.
 */
void SessionDialogBase::onAccept()
{
    accept(); // Subclasses can override
}

/**
 * @brief   SessionDialogBase::onRowClicked Slot called when a row in the session table is clicked.
 * @param   row         The index of the clicked row.
 * 
 * @details This function updates the session name and description fields
 *          based on the selected row in the session table. It also enables
 *          the OK button and updates the session preview widget.
 */
void SessionDialogBase::onRowClicked(int row)
{
    QTableWidgetItem *nameItem = ui->sessionTable->item(row, 0);
    QTableWidgetItem *descItem = ui->sessionTable->item(row, 1);

    if (nameItem)
        ui->sessionNameEdit->setText(nameItem->text());

    if (descItem)
        ui->sessionDescEdit->setText(descItem->text());

    enableOKButton(true);

    Session s = store.getSession(nameItem->text());

    ui->previewWidget->setSession(s);
}

/**
 * @brief   SessionDialogBase::requestDeleteSelected Slot called when the delete button is clicked.
 * 
 * @details This function prompts the user to confirm deletion of the selected session.
 *          If confirmed, it emits a deleteRequested signal with the session name.
 */
void SessionDialogBase::requestDeleteSelected() {

    const QString currentName = ui->sessionNameEdit->text();

    if (currentName.isEmpty())
        return;

    if (QMessageBox::question(this, tr("Delete Session"),
                              tr("Delete session '%1'?").arg(currentName)) == QMessageBox::Yes) {
        emit deleteRequested(currentName);
    }
}

/**
 * @brief   SessionDialogBase::doDelete Deletes the specified session.
 * @param   name        The name of the session to delete.
 * 
 * @details This function deletes the session with the given name from the SessionStore
 *          and repopulates the session table.
 */
void SessionDialogBase::doDelete(const QString &name)
{
    //    if (store.autoStartSession() == name)
    //            store.setAutoStart(QString()); // clear AutoStart if needed
    store.deleteSession(name);
    populateSessionTable();

}
