// /*
//  * Q3270 Terminal Emulator
//  *
//  * Copyright (c) 2025 Andy Styles
//  * SPDX-License-Identifier: BSD-3-Clause
//  *
//  * This file is part of Q3270.
//  * See the LICENSE file in the project root for full license information.
//  */

#ifndef FONTWIDGET_H
#define FONTWIDGET_H

#include <QWidget>
#include <QFontDatabase>
#include <QListWidgetItem>

namespace Ui
{
    class FontWidget;
}


class FontWidget : public QWidget
{
    Q_OBJECT
    
    public:
        
        FontWidget(QWidget *parent = nullptr);
        
        void setFont(QFont font);
        QFont currentFont();
        
    signals:    
        
        void fontChanged(QFont font);
    
    private:
        
        Ui::FontWidget *ui;
        
        QFontDatabase fontDb;
        
        QString lastStyle;
        int lastSize;
        
        void updatePreview();
        
    private slots:
        
        void updateStyles(QListWidgetItem *item);
        void updateSizes();
        
        void sizeChanged(QListWidgetItem *current, QListWidgetItem *last);
        void styleChanged(QListWidgetItem *current, QListWidgetItem *last);
};

#endif // FONTWIDGET_H
