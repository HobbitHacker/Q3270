#include <QFileDialog>

#include "FileTransferDialog.h"

#include "ui_FileTransferDialog.h"


FileTransferDialog::FileTransferDialog(Terminal *terminal, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileTransferDialog)
    , terminal(terminal)

{
    ui->setupUi(this);

/*    connect(ui->sendFileButton, &QPushButton::clicked, this, [this]() {
        terminal->sendFile(ui->fileNameEdit->text());
    });
*/
    connect(ui->browse, &QPushButton::clicked, this, &FileTransferDialog::on_browseButton_clicked);
    connect(ui->direction, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_direction_changed);
    connect(ui->hosttype, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_hosttype_changed);
    connect(ui->recfm, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_recfm_changed);
    connect(ui->space, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_space_changed);
    connect(ui->ascii, &QCheckBox::stateChanged, this, &FileTransferDialog::on_ascii_changed);

    ui->space->setCurrentIndex(0);
    ui->recfm->setCurrentIndex(0);
}

FileTransferDialog::~FileTransferDialog()
{
    delete ui;
}

void FileTransferDialog::showForm()
{
    this->exec();
}

void FileTransferDialog::on_browseButton_clicked()
{
    QString title = ui->direction->currentIndex() == 0 ? tr("Select File to Send")
                                                       : tr("Select File to Receive");

    QString fileName = QFileDialog::getOpenFileName(this, title, QDir::homePath());

    if (!fileName.isEmpty()) {
        ui->localFile->setText(fileName);
    }
}

void FileTransferDialog::on_direction_changed()
{
    setOptions();

    int send = ui->direction->currentIndex() == 0;

    ui->ascii->QAbstractButton::setText(send ? tr("ASCII to EBCDIC translation") : tr("EBCDIC to ASCII translation"));

    ui->recfm->setEnabled(send);
    ui->lrecl->setEnabled(send);
    ui->blksize->setEnabled(send);
    ui->space->setEnabled(send);
    ui->primary->setEnabled(send);
    ui->secondary->setEnabled(send);
    ui->averageblock->setEnabled(send);
}

void FileTransferDialog::setOptions()
{
    Q3270::TransferHostType hosttype = static_cast<Q3270::TransferHostType>(ui->hosttype->currentIndex());

    switch (hosttype) {
        case Q3270::TransferHostType::TSO: // TSO
            ui->recfm->setEnabled(true);
            ui->lrecl->setEnabled(true);
            ui->space->setEnabled(true);
            ui->primary->setEnabled(true);
            ui->secondary->setEnabled(true);
            ui->averageblock->setEnabled(true);

            ui->recfm->clear();
            ui->recfm->addItems({"default", "fixed", "variable", "undefined"});
            break;
        case Q3270::TransferHostType::VM: // VM
            ui->recfm->setEnabled(true);
            ui->lrecl->setEnabled(true);
            ui->space->setEnabled(false);
            ui->primary->setEnabled(false);
            ui->secondary->setEnabled(false);
            ui->averageblock->setEnabled(false);
            ui->recfm->clear();
            ui->recfm->addItems({"default", "fixed", "variable"});
            break;
        case Q3270::TransferHostType::CICS: // CICS
            ui->recfm->setEnabled(false);
            ui->lrecl->setEnabled(false);
            ui->space->setEnabled(false);
            ui->primary->setEnabled(false);
            ui->secondary->setEnabled(false);
            ui->averageblock->setEnabled(false);
            break;
    }

    ui->recfm->setCurrentIndex(0);
}

void FileTransferDialog::on_hosttype_changed()
{
    setOptions();
}

void FileTransferDialog::on_recfm_changed()
{
    QString recfm = ui->recfm->currentText();

    ui->lrecl->setEnabled(recfm != "default");
    ui->blksize->setEnabled(recfm != "default");
}

void FileTransferDialog::on_space_changed()
{
    bool space = ui->space->currentText() == "default";
    bool avblk = ui->space->currentText() == "average blocksize";

    ui->primary->setDisabled(space);
    ui->secondary->setDisabled(space);
    ui->averageblock->setEnabled(avblk);
}

void FileTransferDialog::on_ascii_changed()
{
    ui->crlf->setEnabled(ui->ascii->isChecked());
}
