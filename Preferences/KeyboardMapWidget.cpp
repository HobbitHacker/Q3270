/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

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
        ui->KeyboardMap->setItem(row, 0, new QTableWidgetItem(function));
        ui->KeyboardMap->setItem(row, 1, new QTableWidgetItem(keys.join(", ")));
        ++row;
    });
}

QString KeyboardMapWidget::functionNameForRow(int row) const
{
    if (QTableWidgetItem *it = ui->KeyboardMap->item(row, 0))
        return it->text();

    return {};
}

QStringList KeyboardMapWidget::mappingsForRow(int row) const
{
    if (QTableWidgetItem *it = ui->KeyboardMap->item(row, 1))
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
        QTableWidgetItem *fnItem   = ui->KeyboardMap->item(row, 0);
        QTableWidgetItem *keysItem = ui->KeyboardMap->item(row, 1);

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
                    QKeySequence keySeq(trimmed);
                    map.setKeyMapping(functionName, keySeq);
                }
            }
        }
    }

    return map;
}
