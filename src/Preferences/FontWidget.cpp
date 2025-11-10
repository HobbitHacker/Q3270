// /*
//  * Q3270 Terminal Emulator
//  *
//  * Copyright (c) 2025 Andy Styles
//  * SPDX-License-Identifier: BSD-3-Clause
//  *
//  * This file is part of Q3270.
//  * See the LICENSE file in the project root for full license information.
//  */

#include <QDebug>

#include  "ui_FontWidget.h"
#include "FontWidget.h"

/**
 * @brief   FontWidget - the dialog for font selection
 * @params  parent - the parent widget
 * 
 * @details FontWidget is used to allow the user to select fonts from the installed font list.
 */
FontWidget::FontWidget(QWidget *parent)
   : QWidget(parent)
   , ui(new Ui::FontWidget)
{
   ui->setupUi(this);
   
   qDebug() << "FontWidget init";
   
    // Populate font details
    ui->familyList->clear();
    for (const QString &family : fontDb.families())
        ui->familyList->addItem(family);
   
   ui->preview->setAlignment(Qt::AlignCenter);
   
   connect(ui->familyList, &QListWidget::currentItemChanged, this, &FontWidget::updateStyles);
   connect(ui->sizeList, &QListWidget::currentItemChanged, this, &FontWidget::sizeChanged);
   connect(ui->styleList, &QListWidget::currentItemChanged, this, &FontWidget::styleChanged);
   
   lastStyle = "";
   lastSize = 0;
}

/**
 * @brief   FontWidgett::void FontWidget::updateStyles - update the list of styles available for a given font
 * @params  current - the currently selected font family
 * 
 * @details Update the list of available styles, reported by this font. Sizes are also updated, as well as the 
 *          preview window.
*/
void FontWidget::updateStyles(QListWidgetItem *current)
{
   if (!current)
      return;
   
   QString family = current->text();
   
   ui->styleList->clear();
   
   ui->styleList->addItems(fontDb.styles(family));
   
   int idx = ui->styleList->findItems(lastStyle, Qt::MatchExactly).value(0) 
   
   ? ui->styleList->row(ui->styleList->findItems(lastStyle, Qt::MatchExactly).first()) 
   : -1;
   if (idx >= 0)
      ui->styleList->setCurrentRow(idx);
   else
      ui->styleList->setCurrentRow(0);   
   
   updateSizes();      // repopulate sizes for this family/style
   updatePreview();    // refresh preview after both lists are valid
}

/**
 * @brief   FontWidget::updateSizes - update the list of avalable font sizes
 * 
 * @details The list of available font sizes is updated based on the font selected. 
 */
void FontWidget::updateSizes()
{
   QString family = ui->familyList->currentItem()->text();
   
   QString style  = ui->styleList->currentItem() ? ui->styleList->currentItem()->text() : QString();
   
   ui->sizeList->clear();
   
   QList<int> sizes = fontDb.pointSizes(family, style);
   
   if (sizes.isEmpty())
      sizes = {8,9,10,11,12,14,16,18,20,24};
   
   for (int s : sizes)
      ui->sizeList->addItem(QString::number(s));
   
   QList<QListWidgetItem*> matches = ui->sizeList->findItems(QString::number(lastSize), Qt::MatchExactly);
   
   if (!matches.isEmpty())
      ui->sizeList->setCurrentItem(matches.first());
   else
      ui->sizeList->setCurrentRow(0);
}

/**
 * @brief   FontWidget::updatePreview - update the font preview window
 * 
 * @details Update the preview window with the selected font, style and size. 
 */
void FontWidget::updatePreview()
{
   QString family = ui->familyList->currentItem() ? ui->familyList->currentItem()->text() : QString();
   QString style  = ui->styleList->currentItem() ? ui->styleList->currentItem()->text() : QString();
   int size       = ui->sizeList->currentItem() ? ui->sizeList->currentItem()->text().toInt() : 12;
   
   QFont font(family, size);
   font.setStyleName(style);
   
   ui->preview->setFont(font);
   
   emit fontChanged(font);
}

/**
 * @brief   FontWidget::styleChanged - update the preview when a new style is selected
 * @params  current - the currently selected style
 * @params  previous - the previously seleced style
 * 
 * @details Update the preview text, and store the selected one to maintain the selected one
 *          if possible across font families.
*/
void FontWidget::styleChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
   Q_UNUSED(previous);
   
   if (current)
      lastStyle = current->text();
   
   updatePreview();
}

/**
 * @brief   FontWidget::sizeChanged - update the preview when a new size is selected
 * @params  current - the currently selected size
 * @params  previous - the previously seleced size
 * 
 * @details Update the preview text, and store the selected one to maintain the selected one
 *          if possible across font families.
 */
void FontWidget::sizeChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
   Q_UNUSED(previous);
   
   if (current)
      lastSize = current->text().toInt();
   
   updatePreview();
}

/**
 * @brief   FontWidget::setFont - Set the currently selected font
 * @params  font - the chosen font
 * 
 * @details Try to select the font in the widget. The font family is first selected from the list, 
 *          then the styles and sizes are updated, and finally, the requested font size and style
 *          are matched. If no match is possible, the first item in each list is selected.
 */
void FontWidget::setFont(QFont font)
{
   qDebug() << font.family();
   qDebug() << font.styleName();
   qDebug() << font.pointSize();
   
   QList<QListWidgetItem*> familyMatch = ui->familyList->findItems(font.family(), Qt::MatchExactly);

   if (!familyMatch.isEmpty())
      ui->familyList->setCurrentItem(familyMatch.first());
   else
      ui->familyList->setCurrentRow(0);
   
   updateStyles(ui->familyList->currentItem());
   updateSizes();

   QList<QListWidgetItem*> styleMatch = ui->styleList->findItems(font.styleName(), Qt::MatchExactly);
   QList<QListWidgetItem*> sizeMatch = ui->sizeList->findItems(QString::number(font.pointSize()), Qt::MatchExactly);
   
   qDebug() << familyMatch.size();
   qDebug() << styleMatch.size();
   qDebug() << sizeMatch.size();
   
   if (!styleMatch.isEmpty())
      ui->styleList->setCurrentItem(styleMatch.first());
   else
      ui->styleList->setCurrentRow(0);
   
   if (!sizeMatch.isEmpty())
      ui->sizeList->setCurrentItem(sizeMatch.first());
   else
      ui->sizeList->setCurrentRow(0);
}

/**
 * @brief   FontWidget::currentFont - return the currently selected font
 * @return  The currently selected font.
 */
QFont FontWidget::currentFont()
{
   return(ui->preview->font());
}
