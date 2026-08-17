/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2026 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QPushButton>

#include "FileTransferProgress.h"
#include "ui_FileTransferProgress.h"

/**
 * @brief   FileTransferProgress::FileTransferProgress - display file transfer progress
 * @param   ftDetails - the file transfer details
 * @param   parent - the parent widget
 *
 * @details Display the file transfer progress. This dialog is displayed when a file transfer is in progress.
 *          It shows the number of bytes and blocks transferred, and allows the user to cancel the transfer.
 */
FileTransferProgress::FileTransferProgress(FileTransfer &ftDetails, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FileTransferProgress)
    ,ftDetails(ftDetails)
{
    ui->setupUi(this);

    // Set cancel button
    ftFailure();
}

/**
 * @brief   FileTransferProgress::~FileTransferProgress - destructor
 *
 * @details Destructor for the file transfer progress dialog.
 */
FileTransferProgress::~FileTransferProgress()
{
    delete ui;
}

/**
 * @brief   FileTransferProgress::reset - reset the file transfer progress dialog
 *
 * @details Reset the file transfer progress dialog. This is called when a new file transfer is started.
 */
void FileTransferProgress::reset()
{
    ui->byteCount->setText(QString::number(0));
    ui->blockCount->setText(QString::number(0));

}

/**
 * @brief   FileTransferProgress::updateStats - update the file transfer progress dialog
 *
 * @details Update the file transfer progress dialog. This is called when a new block of data is transferred.
 */
void FileTransferProgress::updateStats()
{
    ui->byteCount->setText(QString::number(ftDetails.getBytes()));
    ui->blockCount->setText(QString::number(ftDetails.getBlocks()));
}

/**
 * @brief   FileTransferProgress::updateMessage - update the message in the file transfer progress dialog
 * @param   message - the message to display
 *
 * @details Update the message in the file transfer progress dialog. This is called when a new message is received from the host.
 */
void FileTransferProgress::updateMessage(const QString &message)
{
    ui->message->setText(message);
}

/**
 * @brief   FileTransferProgress::complete - complete the file transfer progress dialog
 *
 * @details Complete the file transfer progress dialog. This is called when the file transfer is complete.
 */
void FileTransferProgress::complete()
{
    qDebug() << "FileTransferProgress complete";

    ftDetails.setExpected(false);
}


/**
 * @brief   FileTransferProgress::ftSuccess - file transfer successful
 *
 * @details File transfer successful. This is called when the file transfer is successful.
 *          It hides the cancel button and shows the close button.
 */
void FileTransferProgress::ftSuccess()
{
    ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
    ui->buttonBox->button(QDialogButtonBox::Close)->show();
}

/**
 * @brief   FileTransferProgress::ftFailure - file transfer failed
 *
 * @details File transfer failed. This is called when the file transfer fails.
 *          It hides the close button and shows the cancel button.
 */
void FileTransferProgress::ftFailure()
{
    ui->buttonBox->button(QDialogButtonBox::Close)->hide();
    ui->buttonBox->button(QDialogButtonBox::Cancel)->show();
}
