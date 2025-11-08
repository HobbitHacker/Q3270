/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QDebug>

#include "KeyboardMapWidget.h"
#include "ui_KeyboardMapWidget.h"

KeyboardMapWidget::KeyboardMapWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyboardMapWidget)
{
    ui->setupUi(this);

    ui->KeyboardMap->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->KeyboardMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->KeyboardMap, &QTableWidget::itemClicked, this, &KeyboardMapWidget::handleItemClicked);
}

KeyboardMapWidget::~KeyboardMapWidget()
{
    delete ui;
}

void KeyboardMapWidget::setTheme(const KeyboardMap &map)
{
    ui->KeyboardMap->setRowCount(0);

    int row = 0;

    map.forEach([&row, this](const QString &function, const QStringList &keys)
                {
        ui->KeyboardMap->insertRow(row);
        ui->KeyboardMap->setItem(row, 0, new QTableWidgetItem(keys.join(", ")));
        ui->KeyboardMap->setItem(row, 1, new QTableWidgetItem(function));
        ++row;
    });
}

QString KeyboardMapWidget::functionNameForRow(int row) const
{
    if (QTableWidgetItem *it = ui->KeyboardMap->item(row, 1))
        return it->text();

    return {};
}

QStringList KeyboardMapWidget::mappingsForRow(int row) const
{
    if (QTableWidgetItem *it = ui->KeyboardMap->item(row, 0))
        return it->text().split(", ");

    return {};
}

void KeyboardMapWidget::handleItemClicked(QTableWidgetItem *item)
{
    int row = item->row();

    emit mappingClicked(row, functionNameForRow(row), mappingsForRow(row));
}

KeyboardMap KeyboardMapWidget::currentMappings() const
{
    KeyboardMap map;

    for (int row = 0; row < ui->KeyboardMap->rowCount(); ++row) {
        QTableWidgetItem *keysItem = ui->KeyboardMap->item(row, 0);
        QTableWidgetItem *fnItem   = ui->KeyboardMap->item(row, 1);

        if (!fnItem)
            continue;

        const QString functionName = fnItem->text().trimmed();
        if (functionName.isEmpty())
            continue;


        if (keysItem) {
            const QStringList raw = keysItem->text().split(',', Qt::SkipEmptyParts);
            for (const QString &seqStr : raw) {
                const QString trimmed = seqStr.trimmed();
                if (!trimmed.isEmpty()) {
                    map.setKeyMapping(functionName, trimmed);
                }
            }
        }
    }

    return map;
}

