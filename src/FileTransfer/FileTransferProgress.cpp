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
    showAbort();
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
 * @brief   FileTransferProgress::showClose - file transfer successful
 *
 * @details File transfer successful. This is called when the file transfer is successful.
 *          It hides the cancel button and shows the close button.
 */
void FileTransferProgress::showClose()
{
    ui->buttonBox->button(QDialogButtonBox::Abort)->hide();
    ui->buttonBox->button(QDialogButtonBox::Close)->show();
}

/**
 * @brief   FileTransferProgress::showAbort - file transfer failed
 *
 * @details File transfer failed. This is called when the file transfer fails.
 *          It hides the close button and shows the cancel button.
 */
void FileTransferProgress::showAbort()
{
    ui->buttonBox->button(QDialogButtonBox::Close)->hide();
    ui->buttonBox->button(QDialogButtonBox::Abort)->show();
}

/**
 * @brief   FileTransferProgress::setDirection - set the direction of the file transfer
 * @param   direction - the direction of the file transfer
 *
 * @details Set the direction of the file transfer. This is called when a new file transfer is started.
 * 
 */
void FileTransferProgress::setDirection(Q3270::TransferDirection direction)
{
    if (direction == Q3270::TransferDirection::Send)
    {
            setWindowTitle("Sending File");
            ui->blockMessage->setText("Blocks Sent:");
            ui->byteMessage->setText("Bytes Sent:");
    }
    else
    {
            setWindowTitle("Receiving File");
            ui->blockMessage->setText("Blocks Received:");
            ui->byteMessage->setText("Bytes Received:");
    }
}

/**
 * @brief   FileTransferProgress::reject - reject the file transfer
 *
 * @details Reject the file transfer. This is called when the user cancels the file transfer. The
 *          first time this is called, it emits a signal to cancel the transfer, which is when
 *          the user clicks the cancel button. The second time, it closes the dialog, which is
  *         when the user clicks the close button after the cancellation is complete.
 */
void FileTransferProgress::reject()
{
    if (ftDetails.getState() != FileTransfer::FTState::NONE)
    {
        emit cancelTransfer();
    }
    else
    {
        QDialog::reject();
    }
}