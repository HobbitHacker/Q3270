#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include "ui_SessionDialog.h"
#include "ui_SessionPreview.h"
#include "SessionDialogBase.h"
#include "HostAddressUtils.h"

SessionDialogBase::SessionDialogBase(QWidget *parent)
    : QDialog(parent), ui(new Ui::SessionDialog), store()
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

    Session s = store.loadSession(nameItem->text());

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
