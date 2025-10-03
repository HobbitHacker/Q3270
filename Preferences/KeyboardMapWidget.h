/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef KEYBOARDMAPWIDGET_H
#define KEYBOARDMAPWIDGET_H

#include <QWidget>
#include <QTableWidget>

#include "Models/KeyboardMap.h"

namespace Ui
{
    class KeyboardMapWidget;
}

class KeyboardMapWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit KeyboardMapWidget(QWidget *parent = nullptr);
        ~KeyboardMapWidget();

        void setTheme(const KeyboardMap &map);

        QString functionNameForRow(int row) const;
        QStringList mappingsForRow(int row) const;

        KeyboardMap currentMappings() const;

    signals:

        void mappingClicked(int row, const QString &functionName, const QStringList &mappings);

    private slots:
        void handleItemClicked(QTableWidgetItem *item);


    private:
        Ui::KeyboardMapWidget *ui;
};

#endif // KEYBOARDMAPWIDGET_H
