/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef SESSIONPREVIEWWIDGET_H
#define SESSIONPREVIEWWIDGET_H

#include <QWidget>
#include "ui_SessionPreview.h"
#include "Models/Session.h"

class SessionPreviewWidget : public QWidget, private Ui::SessionPreviewWidget
{
    Q_OBJECT
public:
    explicit SessionPreviewWidget(QWidget *parent = nullptr);
    void setSession(const Session &session);
    void clear();
};
#endif // SESSIONPREVIEWWIDGET_H
