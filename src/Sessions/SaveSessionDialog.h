/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef SAVESESSIONDIALOG_H
#define SAVESESSIONDIALOG_H

#include "SessionDialogBase.h"
#include "ActiveSettings.h"

class SaveSessionDialog : public SessionDialogBase
{
    Q_OBJECT

public:
    SaveSessionDialog(SessionStore &store, ActiveSettings &settings, QWidget *parent = nullptr);

private slots:
    void onAccept() override;
    void saveSessionNameEdited(const QString &name);

private:
    ActiveSettings &activeSettings;
};

#endif // SAVESESSIONDIALOG_H
