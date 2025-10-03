/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "LockIndicator.h"
#include <QtSvg>
#include <QDateTime>

LockIndicator::LockIndicator(QGraphicsItem* parent) : QGraphicsItemGroup(parent)
{
    // "X" text for the clock indicator
    xText = new QGraphicsSimpleTextItem("X", this);
    xText->setPen(QPen(Qt::white));
    xText->setVisible(false);

    // SVG pseudo-clock glyph
    clockSvg = new QGraphicsSvgItem(QStringLiteral(":/Icons/clock.svg"), this);
    clockSvg->setVisible(false);

    auto effect = new QGraphicsColorizeEffect;
    effect->setColor(Qt::white);
    clockSvg->setGraphicsEffect(effect);

    QRectF r = clockSvg->boundingRect();
    clockSvg->setScale(7 / r.height());

    QFont statusBarText = QFont("ibm3270");
    statusBarText.setPixelSize(8);
    QFontMetrics fm = QFontMetrics(statusBarText);

    // "X System" text
    systemText = new QGraphicsSimpleTextItem("X System", this);
    systemText->setPen(QPen(Qt::white));
    systemText->setVisible(false);

    systemText->setFont(statusBarText);
    xText->setFont(statusBarText);

    // Add to group so transformations apply uniformly
    addToGroup(xText);
    addToGroup(clockSvg);
    addToGroup(systemText);

    // Positioning: place the SVG just to the right of the "X"
    clockSvg->setPos(xText->boundingRect().width() + 2, 2);

    currentMode = None;
}

void LockIndicator::setMode(Mode m)
{
    currentMode = m;

    // Hide everything first
    xText->setVisible(false);
    clockSvg->setVisible(false);
    systemText->setVisible(false);

    switch (currentMode) {
        case Clock:
            xText->setVisible(true);
            clockSvg->setVisible(true);
            break;
        case System:
            systemText->setVisible(true);
            break;
        case None:
        default:
            break;
    }
}

LockIndicator::Mode LockIndicator::mode() const
{
    return currentMode;
}

void LockIndicator::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
{
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "LockIndicator   : Paint called with mode " << currentMode;
}
