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

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include "ui_SessionDialog.h"
#include "ui_SessionPreview.h"
#include "SessionDialogBase.h"
#include "HostAddressUtils.h"

SessionDialogBase::SessionDialogBase(SessionStore &store, QWidget *parent)
    : QDialog(parent), ui(new Ui::SessionDialog), store(store)
{
    ui->setupUi(this);

    sessions = store.listSessions();

    setupTable();
    connectSignals();

    populateSessionTable();
}

SessionDialogBase::~SessionDialogBase()
{
    delete ui;
}

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

void SessionDialogBase::connectSignals()
{
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SessionDialogBase::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &SessionDialogBase::reject);
    connect(ui->deleteButton, &QPushButton::clicked, this, &SessionDialogBase::requestDeleteSelected);
    connect(ui->sessionTable, &QTableWidget::cellClicked, this, &SessionDialogBase::onRowClicked);
}

void SessionDialogBase::enableOKButton(bool enabled)
{
    if (ui->buttonBox)
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

void SessionDialogBase::setOKButtonText(const QString &text)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(text);
}

void SessionDialogBase::onAccept()
{
    accept(); // Subclasses can override
}

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

void SessionDialogBase::requestDeleteSelected() {

    const QString currentName = ui->sessionNameEdit->text();

    if (currentName.isEmpty())
        return;

    if (QMessageBox::question(this, tr("Delete Session"),
                              tr("Delete session '%1'?").arg(currentName)) == QMessageBox::Yes) {
        emit deleteRequested(currentName);
    }
}


void SessionDialogBase::doDelete(const QString &name)
{
    //    if (store.autoStartSession() == name)
    //            store.setAutoStart(QString()); // clear AutoStart if needed
    store.deleteSession(name);
    populateSessionTable();

}
