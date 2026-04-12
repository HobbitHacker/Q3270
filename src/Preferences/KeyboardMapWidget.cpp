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

/**
 * @brief   KeyboardMapWidget::KeyboardMapWidget - widget to display the keyboard map
 * @param   parent - the parent widget
 *
 * @details Initialise the widget and set up the table to display the keyboard map.
 */
KeyboardMapWidget::KeyboardMapWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyboardMapWidget)
{
    ui->setupUi(this);

    ui->KeyboardMap->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->KeyboardMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->KeyboardMap, &QTableWidget::itemClicked, this, &KeyboardMapWidget::handleItemClicked);
}

/**
 * @brief   KeyboardMapWidget::~KeyboardMapWidget - destructor
 *
 * @details Clean up the widget.
 */
KeyboardMapWidget::~KeyboardMapWidget()
{
    delete ui;
}

/**
 * @brief   KeyboardMapWidget::setTheme - set the keyboard map to display
 * @param   map - the keyboard map to display
 *
 * @details setTheme takes a keyboard map and sets up the mappings of keys to function names in the table.
 */
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

/**
 * @brief   KeyboardMapWidget::functionNameForRow - get the function name for a given row
 * @param   row - the row to get the function name for
 * @return  the function name for the given row, or an empty string if not found
 *
 * @details Helper to get the function name for a given row in the table.
 */
QString KeyboardMapWidget::functionNameForRow(int row) const
{
    if (QTableWidgetItem *it = ui->KeyboardMap->item(row, 1))
        return it->text();

    return {};
}

/**
 * @brief   KeyboardMapWidget::mappingsForRow - get the key mappings for a given row
 * @param   row - the row to get the key mappings for
 * @return  the key mappings for the given row, or an empty list if not found
 *
 * @details Helper to get the key mappings for a given row in the table.
 */
QStringList KeyboardMapWidget::mappingsForRow(int row) const
{
    if (QTableWidgetItem *it = ui->KeyboardMap->item(row, 0))
        return it->text().split(", ");

    return {};
}

/**
 * @brief   KeyboardMapWidget::handleItemClicked - handle a click on an item in the table
 * @param   item - the item that was clicked
 *
 * @details When an item is clicked in the table, emit a signal with the function name and key
 *          mappings for that row. Used by the Preferences dialog to allow the user to edit the
 *          mappings for a function.
 */
void KeyboardMapWidget::handleItemClicked(QTableWidgetItem *item)
{
    int row = item->row();

    emit mappingClicked(row, functionNameForRow(row), mappingsForRow(row));
}

/**
 * @brief   KeyboardMapWidget::currentMappings - get the current keyboard map from the table
 * @return  the current keyboard map from the table
 *
 * @details currentMappings reads the current state of the table and constructs a KeyboardMap object
 *          representing the mappings currently displayed in the table. Used by the Preferences dialog
 *          to save changes made by the user to the keyboard map.
 */
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

