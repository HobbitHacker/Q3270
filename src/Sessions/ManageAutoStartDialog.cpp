/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "ManageAutoStartDialog.h"
#include "ui_ManageAutoStartDialog.h"

/**
 * @brief   ManageAutoStartDialog::ManageAutoStartDialog constructor.
 * @param   store               Reference to the SessionStore.
 * @param   parent              Parent widget.
 * 
 * @details This dialog allows the user to manage sessions that are set to auto-start.
 */
ManageAutoStartDialog::ManageAutoStartDialog(SessionStore &store, QWidget *parent)
    : QDialog(parent),
    ui(new Ui::ManageAutoStartDialog),
    store(store)
{
    ui->setupUi(this);
    setWindowTitle(tr("Manage AutoStart Sessions"));

    connect(ui->addButton, &QPushButton::clicked, this, &ManageAutoStartDialog::handleAddButtonClicked);
    connect(ui->removeButton, &QPushButton::clicked, this, &ManageAutoStartDialog::handleRemoveButtonClicked);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ManageAutoStartDialog::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(ui->availableList, &QListWidget::itemClicked, this, &ManageAutoStartDialog::onAvailableRowClicked);
    connect(ui->autoStartList, &QListWidget::itemClicked, this, &ManageAutoStartDialog::onAutoStartRowClicked);

    allSessions = store.listSessions();
    autoStartSessions = store.listAutoStartSessions();

    refreshLists();
}

/**
 * @brief   ManageAutoStartDialog destructor.
 * 
 * @note    Probably not needed as Qt parent-child system handles deletion of child widgets.
 */
ManageAutoStartDialog::~ManageAutoStartDialog()
{
    delete ui;
}

/**
 * @brief   Refresh the available and auto-start session lists in the UI.
 * 
 * @details This function clears both lists and repopulates them based on the
 *          current state of all sessions and the auto-start sessions.
 */
void ManageAutoStartDialog::refreshLists()
{
    ui->availableList->clear();
    ui->autoStartList->clear();

    for (const Session &session : allSessions) {
        QListWidget *targetList = autoStartSessions.contains(session.name)
                                      ? ui->autoStartList
                                      : ui->availableList;

        new QListWidgetItem(session.name, targetList);
    }
}

/**
 * @brief   Handle the Add button click event.
 * 
 * @details This function adds the selected sessions from the available list
 *          to the auto-start sessions and refreshes the lists.
 */
void ManageAutoStartDialog::handleAddButtonClicked()
{
    QList<QListWidgetItem *> selected = ui->availableList->selectedItems();
    for (int i = 0; i < selected.size(); ++i) {
        autoStartSessions.append(selected.at(i)->text());
    }
    refreshLists();
    ui->availablePreview->clear();
}

/**
 * @brief   Handle the Remove button click event.
 * 
 * @details This function removes the selected sessions from the auto-start list
 *          and refreshes the lists.
 */
void ManageAutoStartDialog::handleRemoveButtonClicked()
{
    QList<QListWidgetItem *> selected = ui->autoStartList->selectedItems();
    for (int i = 0; i < selected.size(); ++i) {
        autoStartSessions.removeOne(selected.at(i)->text());
    }
    refreshLists();
    ui->autoStartPreview->clear();
}

/**
 * @brief   Handle click event on an available session row.
 * @param   item    The clicked QListWidgetItem.
 * 
 * @details This function updates the preview widget to show details
 *          of the selected available session.
 */
void ManageAutoStartDialog::onAvailableRowClicked(QListWidgetItem *item)
{
    const Session s = store.getSession(item->text());
    ui->availablePreview->setSession(s);
}

/**
 * @brief   Handle click event on an auto-start session row.
 * @param   item    The clicked QListWidgetItem.
 * 
 * @details This function updates the preview widget to show details
 *          of the selected auto-start session.
 */
void ManageAutoStartDialog::onAutoStartRowClicked(QListWidgetItem *item)
{
    const Session s = store.getSession(item->text());
    ui->autoStartPreview->setSession(s);
}

/**
 * @brief   Handle the dialog acceptance event.
 * 
 * @details This function saves the updated list of auto-start sessions
 *          to the SessionStore and accepts the dialog.
 */
void ManageAutoStartDialog::onAccept()
{
    store.saveAutoStartSessions(autoStartSessions);
    accept();
}
