#include <QFileDialog>
#include <QMessageBox>

#include "FileTransferDialog.h"

#include "ui_FileTransferDialog.h"


FileTransferDialog::FileTransferDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileTransferDialog)

{
    ui->setupUi(this);

    connect(ui->browse, &QPushButton::clicked, this, &FileTransferDialog::on_browseButton_clicked);
    connect(ui->direction, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_direction_changed);
    connect(ui->hosttype, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_hosttype_changed);
    connect(ui->recfm, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_recfm_changed);
    connect(ui->space, &QComboBox::currentIndexChanged, this, &FileTransferDialog::on_space_changed);
    connect(ui->ascii, &QCheckBox::toggled, this, &FileTransferDialog::on_ascii_changed);

    ui->space->setCurrentIndex(0);
    ui->recfm->setCurrentIndex(0);
}

FileTransferDialog::~FileTransferDialog()
{
    delete ui;
}

bool FileTransferDialog::showForm()
{
    for (;;) {
        int result = this->exec();

        if (result == QDialog::Rejected)
            return false;

        qDebug() << "FileTransferDialog::showForm() - local file:" << getLocalFile() << "remote file:" << ui->remoteFile->text() << "direction:" << getDirection();

        if (getDirection() == Q3270::TransferDirection::Send)
            break;

        qDebug() << QFile::exists(getLocalFile());

        if (QFile::exists(getLocalFile())) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "File Exists", "The file already exists. Do you want to overwrite it?", QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                break;
            }
        }
        else
            break;
    }

    return true;
}

const QString FileTransferDialog::getCommand()
{
    QString command = "IND$FILE ";

    if (ui->direction->currentIndex() == 0) {
        command.append("PUT ");
    } else {
        command.append("GET ");
    }

    command.append(ui->remoteFile->text()).append(" ");

    Q3270::TransferHostType hosttype = static_cast<Q3270::TransferHostType>(
        ui->hosttype->currentIndex());

    // Non-TSO hosts get options after a parenthesis
    if (hosttype != Q3270::TransferHostType::TSO)
        command.append("( ");

    // Append ASCII if checked; otherwise for CICS explicitly add BINARY
    if (ui->ascii->isChecked()) {
        command.append("ASCII ");
        if (ui->crlf->isChecked()) {
            command.append("CRLF ");
        }
    } else if (hosttype == Q3270::TransferHostType::CICS) {
        command.append("EBCDIC NOCRLF ");
    }

    if (ui->append->isChecked()) {
        command.append("APPEND ");
    }

    switch (hosttype) {
        case Q3270::TransferHostType::TSO: // TSO
            if (ui->recfm->currentText() != "default") {
                command.append("RECFM(").append(ui->recfm->currentText()[0]).append(") ");

                // TODO: x3270 omits parms if the values are zero
                if (!ui->lrecl->text().isEmpty())
                    command.append("LRECL(").append(ui->lrecl->text()).append(") ");

                if (!ui->blksize->text().isEmpty())
                    command.append("BLKSIZE(").append(ui->blksize->text()).append(") ");
            }
            if (ui->space->currentText() != "default") {
                command.append("SPACE(").append(ui->primary->text());

                if (!ui->secondary->text().isEmpty())
                    command.append(",").append(ui->secondary->text());

                command.append(") ");

                if (ui->space->currentText() == "average blocksize")
                    command.append("AVBLOCK(").append(ui->averageblock->text()).append(") ");
                else if (ui->space->currentText() == "tracks")
                    command.append("TRACKS");
                else
                    command.append("CYLINDERS");
            }
            break;
        case Q3270::TransferHostType::VM: // VM
            command.append("VM ");
            break;
        case Q3270::TransferHostType::CICS: // CICS
            command.append("CICS ");
            break;
    }
    return command.trimmed();
}

void FileTransferDialog::on_browseButton_clicked()
{
    bool send = ui->direction->currentIndex() == 0;

    QString title = send ? tr("Select File to Send") : tr("Select File to Receive");

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

Q3270::TransferDirection FileTransferDialog::getDirection()
{
    return ui->direction->currentIndex() == 0 ? Q3270::TransferDirection::Send :
                                                Q3270::TransferDirection::Receive;
}

QString FileTransferDialog::getLocalFile()
{
    QString fileName = ui->localFile->text();

    if (fileName.startsWith("~")) {
        fileName = QDir::homePath() + fileName.mid(1);
    }

    return fileName;
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

void FileTransferDialog::on_dftEnabled_changed()
{
    bool dftEnabled = ui->dftEnabled->isChecked();

    ui->dftBlockSize->setEnabled(dftEnabled);
}

bool FileTransferDialog::isAscii()
{
    return ui->ascii->isChecked();
}

bool FileTransferDialog::isCRLF()
{
    return ui->crlf->isChecked();
}

bool FileTransferDialog::isDFTEnabled()
{
    bool dftEnabled = ui->dftEnabled->isChecked();
    qDebug() << "FileTransferDialog::isDFTEnabled() - dftEnabled:" << dftEnabled;
    return ui->dftEnabled->isChecked();
}

quint16 FileTransferDialog::getDFTBlockSize()
{
    return ui->dftBlockSize->text().toInt();
}