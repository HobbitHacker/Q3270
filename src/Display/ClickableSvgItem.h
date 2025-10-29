/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef CLICKABLESVGITEM_H
#define CLICKABLESVGITEM_H

#include <QGraphicsSvgItem>

class ClickableSvgItem : public QGraphicsSvgItem
{
    public:
        using QGraphicsSvgItem::QGraphicsSvgItem;

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

};

#endif // CLICKABLESVGITEM_H
