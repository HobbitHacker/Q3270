/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef OPENSESSIONDIALOG_H
#define OPENSESSIONDIALOG_H

#include "SessionDialogBase.h"

class OpenSessionDialog : public SessionDialogBase
{
    Q_OBJECT

    public:
        explicit OpenSessionDialog(SessionStore &store, ActiveSettings &activeSettings, QWidget *parent = nullptr);

    private:

        ActiveSettings &activeSettings;

    private slots:

        void onOpenClicked();
};

#endif
