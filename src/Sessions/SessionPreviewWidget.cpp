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

/**
 * @brief   SessionPreviewWidget::SessionPreviewWidget constructor.
 * @param   parent      Parent widget.
 *
 * @details This widget provides a preview of a session's settings.
 */
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

/**
 * @brief   SessionPreviewWidget::setSession Sets the session to preview.
 * @param   session     The session to preview.
 *
 * @details This function updates the preview widget with the details of the given session.
 */
void SessionPreviewWidget::setSession(const Session &session)
{
    groupBox->setEnabled(true);

    previewAddress->setText(HostAddressUtils::format(session.hostName, session.hostPort, session.hostLU));
    previewModel->setText(session.terminalModel);
    previewSecure->setPixmap(session.secureConnection ? checked : unchecked);
    previewVerifyCert->setPixmap(session.verifyCertificate ? checked : unchecked);
}

/**
 * @brief   SessionPreviewWidget::clear Clears the session preview.
 *
 * @details This function resets the preview widget to its default state.
 */
void SessionPreviewWidget::clear()
{
    groupBox->setEnabled(false);

    previewAddress->clear();
    previewModel->clear();
    previewSecure->setPixmap(unchecked);
    previewVerifyCert->setPixmap(unchecked);
}
