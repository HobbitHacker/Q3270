#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

#include "ui_SessionDialog.h"
#include "SessionDialogBase.h"
#include "HostAddressUtils.h"

SessionDialogBase::SessionDialogBase(Mode mode, QWidget *parent)
    : QDialog(parent), ui(new Ui::SessionDialog), mode(mode), store()
{
    ui->setupUi(this);

    sessions = store.listSessions();

    setupTable();
    connectSignals();

    ui->groupBox->setEnabled(false);

    ui->previewSecure->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    ui->previewVerifyCert->setAttribute(Qt::WA_TransparentForMouseEvents, true);

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
    ui->sessionTable->horizontalHeader()->setStretchLastSection(true); // Or use setSectionResizeMode in code
}

void SessionDialogBase::populateSessionTable()
{
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

    qDebug() << "Clicked:" << nameItem->text();

    updatePreview(s);
}

void SessionDialogBase::updatePreview(const Session &s)
{
    ui->groupBox->setEnabled(true);

    qDebug() << s.name;
    qDebug() << s.hostName << s.hostPort << s.hostLU;
    qDebug() << HostAddressUtils::format(s.hostName, s.hostPort, s.hostLU);

    ui->previewAddress->setText(HostAddressUtils::format(s.hostName, s.hostPort,s.hostLU));
    ui->previewModel->setText(s.terminalModel);
    ui->previewSecure->setChecked(s.secureConnection);
    ui->previewVerifyCert->setChecked(s.verifyCertificate);
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
