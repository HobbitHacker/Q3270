/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "ClickableSvgItem.h"

#include <QDebug>

/**
 * @brief   ClickableSvgItem::mousePressEvent - a clickable SVG icon
 * @param   event - the mouse event that triggered this call
 *
 * @details This is used when the user clicks on the padlock icon in the status bar.
 *
 * @note    This is currently only used to show that we can react to users clicking the
 *          padlock. It will, in future, be used to emit a signal that will percolate up
 *          to the MainWindow layer to display certificate details.
 */
void ClickableSvgItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsSvgItem::mousePressEvent(event);
    qDebug() << "Mouse clicked on ClickableSvgItem";
}
