/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "SessionPreviewWidget.h"
#include "HostAddressUtils.h"

SessionPreviewWidget::SessionPreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    clear();

    previewSecure->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    previewVerifyCert->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void SessionPreviewWidget::setSession(const Session &session)
{
    groupBox->setEnabled(true);

    previewAddress->setText(HostAddressUtils::format(session.hostName, session.hostPort, session.hostLU));
    previewModel->setText(session.terminalModel);
    previewSecure->setChecked(session.secureConnection);
    previewVerifyCert->setChecked(session.verifyCertificate);
}

void SessionPreviewWidget::clear()
{
    groupBox->setEnabled(false);

    previewAddress->clear();
    previewModel->clear();
    previewSecure->setChecked(false);
    previewVerifyCert->setChecked(false);
}
