#include "FileTransferProgress.h"
#include "ui_FileTransferProgress.h"

FileTransferProgress::FileTransferProgress(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileTransferProgress)
{
    ui->setupUi(this);
}

FileTransferProgress::~FileTransferProgress()
{
    delete ui;
}

void FileTransferProgress::reset()
{
    ui->byteCount->setText(QString::number(0));
    ui->blockCount->setText(QString::number(0));
}

void FileTransferProgress::updateStats(const unsigned long long bytesReceived,
                                       const unsigned long long blocksReceived)
{
    ui->byteCount->setText(QString::number(bytesReceived));
    ui->blockCount->setText(QString::number(blocksReceived));
}

void FileTransferProgress::updateMessage(const QString &message)
{
    ui->message->setText(message);
}

void FileTransferProgress::complete()
{
    qDebug() << "FileTransferProgress complete";
}
