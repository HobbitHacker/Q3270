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
#include <QStyleOptionButton>

SessionPreviewWidget::SessionPreviewWidget(QWidget *parent)
    : QWidget(parent), check(":/Icons/check-square.svg"), uncheck(":/Icons/x-square.svg")
{
    setupUi(this);
    clear();

    QStyleOptionButton opt;
    QSize indicatorSize = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &opt).size();

    checked = check.pixmap(indicatorSize);
    unchecked = uncheck.pixmap(indicatorSize);

    previewSecure->setPixmap(unchecked);
    previewVerifyCert->setPixmap(unchecked);

    previewSecure->setFixedSize(indicatorSize);
    previewVerifyCert->setFixedSize(indicatorSize);
}

void SessionPreviewWidget::setSession(const Session &session)
{
    groupBox->setEnabled(true);

    previewAddress->setText(HostAddressUtils::format(session.hostName, session.hostPort, session.hostLU));
    previewModel->setText(session.terminalModel);
    previewSecure->setPixmap(session.secureConnection ? checked : unchecked);
    previewVerifyCert->setPixmap(session.verifyCertificate ? checked : unchecked);
}

void SessionPreviewWidget::clear()
{
    groupBox->setEnabled(false);

    previewAddress->clear();
    previewModel->clear();
    previewSecure->setPixmap(unchecked);
    previewVerifyCert->setPixmap(unchecked);
}
