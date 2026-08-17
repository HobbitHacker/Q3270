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
        void complete();

    public slots:
        void updateStats();
        void updateMessage(const QString &message);

        void ftSuccess();
        void ftFailure();

    private:

        Ui::FileTransferProgress *ui;

        FileTransfer &ftDetails;

};

#endif // FILETRANSFERPROGRESS_H
