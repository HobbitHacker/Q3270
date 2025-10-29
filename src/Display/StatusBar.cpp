/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QSvgRenderer>
#include <QGraphicsColorizeEffect>
#include <QDebug>

#include "StatusBar.h"

/**
 * @brief   StatusBar::StatusBar - The 3270 status bar
 * @param   screen_x - width of the initial bar
 * @param   screen_y - height of the initial bar
 *
 * @details StatusBar is responsible for the display of the status bar. This contains:
 *          - the connection state (4-A is hard-coded)
 *          - the security of the connection, represented by varying padlocks
 *          - X<clock> and X System
 *          - the insert status
 *          - the cursor position
 */
StatusBar::StatusBar(int screen_x, int screen_y)
    : screen_x(screen_x)
    , screen_y(screen_y)
    , fm(QFontMetrics(QFont("Courier")))

{
    QColor blue(0x80, 0x80, 0xFF);

    blueLine.setParentItem(this);
    blueLine.setPos(0, 0);
    blueLine.setPen(blue);

    statusBarText = QFont("Sans serif");
    statusBarText.setPixelSize(16);

    fm = QFontMetrics(statusBarText);

    unlock = new ClickableSvgItem(":/Icons/unlock.svg", this);
    unlock->setToolTip("Unsecured Connection");

    lock = new ClickableSvgItem(":/Icons/lock.svg", this);
    lock->setToolTip("Secured Connection, but certificate chain is not secure");

    locktick = new ClickableSvgItem(":/Icons/lock-tick.svg", this);
    locktick->setToolTip("Secured Connection");

    clock = new QGraphicsSvgItem(QStringLiteral(":/Icons/clock.svg"), this);
    clock->setVisible(false);

    QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect;
    effect->setColor(Qt::white);
    clock->setGraphicsEffect(effect);

    QRectF r = lock->boundingRect();

    for (ClickableSvgItem *item : { unlock, lock, locktick }) {
        item->setPos(screen_x * 0.05, 4);
        item->setVisible(false);
        item->setScale(16 / r.height());
    }

    // X <clock>
    xText.setText("X");
    xText.setParentItem(this);
    xText.setBrush(QColor(Qt::white));
    xText.setVisible(false);

    // X System
    xSystemText.setText("X System");
    xSystemText.setParentItem(this);
    xSystemText.setBrush(QColor(Qt::white));
    xSystemText.setVisible(false);

    xSystemText.setFont(statusBarText);
    xText.setFont(statusBarText);

    statusConnect.setBrush(blue);
    statusConnect.setFont(statusBarText);
    statusConnect.setParentItem(this);
    statusConnect.setText("4-A");

    statusCursor.setBrush(blue);
    statusCursor.setFont(statusBarText);
    statusCursor.setParentItem(this);
    statusCursor.setText("0,0");

    statusInsert.setBrush(blue);
    statusInsert.setFont(statusBarText);
    statusInsert.setParentItem(this);
    statusInsert.setText("");

    insert   = Q3270::OvertypeMode;

    setWidth(screen_x);
}

/**
 * @brief   StatusBar::setWidth - adjust the positions of the status indicators
 * @param   width - the width of the display matrix
 *
 * @details setWidth adjusts the positions of the status indicators according to the width of the display.
 *          The connection (4-A) is always at 0
 *          The padlock (secure connection etc) is at 5% across
 *          XSystem etc at 20% across
 *          Insert at 40%
 *          Cursor position at 90%
 */
void StatusBar::setWidth(const int width)
{
    screen_x = width;

    blueLine.setLine(0, 0.5, screen_x, 0.5);

    statusConnect.setPos(0, FONT_OFFSET);

    for (ClickableSvgItem *item : { unlock, lock, locktick }) {
        item->setPos(screen_x * 0.05, 4);
    }

    xSystemText.setPos(screen_x * .20, FONT_OFFSET);
    xText.setPos(screen_x * .20, FONT_OFFSET);
    clock->setPos(xText.boundingRect().width() + 2 + screen_x * .20, 4);

    statusInsert.setPos(screen_x * .40, FONT_OFFSET);

    statusCursor.setPos(screen_x * .90, FONT_OFFSET);
}

/**
 * @brief  StatusBar::boundingRect
 * @return The size of the status bar, as passed at construction.
 */
QRectF StatusBar::boundingRect() const
{
    return QRectF(0, 0, screen_x, screen_y);
}

/**
 * @brief   StatusBar::cursorMoved - refresh the cursor position.
 * @param   x - new cursor x position
 * @param   y - new cursor y position
 *
 * @details Called when the cursor moves, so the status bar position can be updated.
 */
void StatusBar::cursorMoved(int x, int y)
{
    statusCursor.setText(QString("%1,%2").arg(y).arg(x));
}

/**
 * @brief   StatusBar::setStatusXSystem - set XSystem text
 * @param   status - Q3270::Unlocked, Q3270::SystemLock or Q3270::TerminalWait
 *
 * @details Called when XSystem / X<clock> is to be shown or removed.
 */
void StatusBar::setStatusLock(Q3270::Indicators status)
{
    xText.setVisible(false);
    clock->setVisible(false);
    xSystemText.setVisible(false);

    switch(status)
    {
        case Q3270::TerminalWait:
            xText.setVisible(true);
            clock->setVisible(true);
            break;
        case Q3270::SystemLock:
            xSystemText.setVisible(true);
            break;
        case Q3270::Unlocked:
            break;
    }
}

/**
 * @brief   StatusBar::setStatusInsert - the Insert mode text
 * @param   ins - Q3270::InsertMode or Q3270::OvertypeMode
 *
 * @details Called to show the Insert status on the status line.
 */
void StatusBar::setStatusInsert(Q3270::Indicators insert)
{
    this->insert = insert;

    statusInsert.setText(insert == Q3270::InsertMode ? QString("\uFF3E") : QString(""));
}

/**
 * @brief  StatusBar::setEncrypted - change encryption status bar icon
 * @param  encrypted - the encryption state
 *
 *        encrypted  | Description
 *        ---------- | -----------
 *          0        | Unencrypted
 *          1        | Encrypted without validating certs
 *          2        | Encrypted
 */
void StatusBar::setEncrypted(Q3270::Encryption encrypted)
{
    unlock->setVisible(encrypted == Q3270::Unencrypted);
    lock->setVisible(encrypted == Q3270::SemiEncrypted);
    locktick->setVisible(encrypted == Q3270::Encrypted);
}


void StatusBar::paint(QPainter *p, const QStyleOptionGraphicsItem *i, QWidget *w)
{
}
