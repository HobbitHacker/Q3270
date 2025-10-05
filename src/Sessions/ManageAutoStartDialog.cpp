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

ManageAutoStartDialog::~ManageAutoStartDialog()
{
    delete ui;
}

void ManageAutoStartDialog::refreshLists()
{
    ui->availableList->clear();
    ui->autoStartList->clear();

    for (const Session &session : allSessions) {
        QListWidget *targetList = autoStartSessions.contains(session.name)
                                      ? ui->autoStartList
                                      : ui->availableList;

        new QListWidgetItem(session.name, targetList);
//        item->setData(Qt::UserRole, QVariant::fromValue(session));
    }
}

void ManageAutoStartDialog::handleAddButtonClicked()
{
    QList<QListWidgetItem *> selected = ui->availableList->selectedItems();
    for (int i = 0; i < selected.size(); ++i) {
//        Session session = selected.at(i)->data(Qt::UserRole).value<Session>();
        autoStartSessions.append(selected.at(i)->text());

    }
    refreshLists();
    ui->availablePreview->clear();
}

void ManageAutoStartDialog::handleRemoveButtonClicked()
{
    QList<QListWidgetItem *> selected = ui->autoStartList->selectedItems();
    for (int i = 0; i < selected.size(); ++i) {
//        Session session = selected.at(i)->data(Qt::UserRole).value<Session>();
        autoStartSessions.removeOne(selected.at(i)->text());
    }
    refreshLists();
    ui->autoStartPreview->clear();
}

void ManageAutoStartDialog::onAvailableRowClicked(QListWidgetItem *item)
{
    const Session s = store.getSession(item->text());
    ui->availablePreview->setSession(s);
}

void ManageAutoStartDialog::onAutoStartRowClicked(QListWidgetItem *item)
{
    const Session s = store.getSession(item->text());
    ui->autoStartPreview->setSession(s);
}


void ManageAutoStartDialog::onAccept()
{
    store.saveAutoStartSessions(autoStartSessions);
    accept();
}
