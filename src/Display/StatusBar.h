/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QGraphicsItem>
#include <QFont>
#include <QFontMetrics>

#include "Q3270.h"
#include "ClickableSvgItem.h"

#define FONT_OFFSET -3

class StatusBar : public QGraphicsObject
{
    Q_OBJECT

    public:

        StatusBar(int screen_x, int screen_y);

        void setStatusInsert(Q3270::Indicators insert);
        void setStatusLock(Q3270::Indicators status);

        void setSize(const int x, const int y);

        QRectF boundingRect() const override;
        void paint(QPainter *p, const QStyleOptionGraphicsItem *i, QWidget *w) override;

    public slots:

        void setEncrypted(Q3270::Encryption e);
        void cursorMoved(int x, int y);

    private:

        int screen_x;
        int screen_y;

        Q3270::Indicators insert;

        int cursor_x;
        int cursor_y;

        qreal iconPosY;
        qreal iconScale;

        QFont statusBarText;
        QFontMetrics fm;

        QGraphicsSimpleTextItem statusConnect;
        QGraphicsSimpleTextItem statusCursor;
        QGraphicsSimpleTextItem statusInsert;

        // Padlocks
        ClickableSvgItem *locktick;
        ClickableSvgItem *unlock;
        ClickableSvgItem *lock;

        // Clock
        QGraphicsSvgItem *clock;

        QGraphicsSimpleTextItem xText;
        QGraphicsSimpleTextItem xSystemText;

        // Status bar thin line
        QGraphicsLineItem blueLine;
};

#endif // STATUSBAR_H
