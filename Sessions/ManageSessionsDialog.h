/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef MANAGESESSIONSDIALOG_H
#define MANAGESESSIONSDIALOG_H

#include "SessionDialogBase.h"

class ManageSessionsDialog : public SessionDialogBase
{
    Q_OBJECT
public:
    explicit ManageSessionsDialog(SessionStore &store, QWidget *parent = nullptr);

private slots:

    void onManageAutoStartClicked();

private:
    QPushButton *autoStartButton;
};

#endif // MANAGESESSIONSDIALOG_H
