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
