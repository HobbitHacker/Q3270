/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2026 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */


#ifndef FILETRANSFERPROGRESS_H
#define FILETRANSFERPROGRESS_H

#include <QDialog>
#include <QElapsedTimer>
#include <QTimer>

#include "FileTransfer/FileTransfer.h"

namespace Ui{
    class FileTransferProgress;
}

class FileTransferProgress : public QDialog
{
    Q_OBJECT

    public:
        explicit FileTransferProgress(FileTransfer &ftDetails, QWidget *parent = nullptr);
        ~FileTransferProgress();

        void reset();
        void setDirection(Q3270::TransferDirection direction, QString fromFile, QString toFile);
        void showClose();
        void showAbort();

    public slots:

        void updateStats();
        void updateMessage(const QString &message);

        void reject();

    private slots:
        void cancelTimedOut();

    signals:

        void cancelTransfer();

    private:

        Ui::FileTransferProgress *ui;

        FileTransfer &ftDetails;
        QTimer cancelTimer;
        QTimer statsTimer;
        QElapsedTimer elapsedTimer;
        qint64 finalElapsedMs = 0;
        bool timingActive = false;
        bool cancelPending = false;
        bool closeAnywayAvailable = false;

};

#endif // FILETRANSFERPROGRESS_H
