/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QColorDialog>

#include "ColourSwatchWidget.h"
#include "ui_ColourSwatchWidget.h"

ColourSwatchWidget::ColourSwatchWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ColourSwatchWidget)
{
    ui->setupUi(this);

    // Build list of buttons for simpler manipulation later
    colourButtons[Q3270::UnprotectedNormal]     = ui->baseUnprotected;
    colourButtons[Q3270::ProtectedNormal]       = ui->baseProtected;
    colourButtons[Q3270::UnprotectedIntensified]= ui->baseUnprotectedIntensify;
    colourButtons[Q3270::ProtectedIntensified]  = ui->baseProtectedIntensify;

    colourButtons[Q3270::Black]   = ui->colourBlack;
    colourButtons[Q3270::Blue]    = ui->colourBlue;
    colourButtons[Q3270::Red]     = ui->colourRed;
    colourButtons[Q3270::Magenta] = ui->colourPink;
    colourButtons[Q3270::Green]   = ui->colourGreen;
    colourButtons[Q3270::Cyan]    = ui->colourTurq;
    colourButtons[Q3270::Yellow]  = ui->colourYellow;
    colourButtons[Q3270::Neutral] = ui->colourWhite;

    for (QToolButton *btn : colourButtons)
    {
        connect(btn, &QToolButton::clicked, this, &ColourSwatchWidget::setColour);
    }
}

void ColourSwatchWidget::setReadOnly(bool ro)
{
    // Make all QToolButtons inside this widget "read-only"
    const QList<QToolButton *>buttons = findChildren<QToolButton *>();

    for (QToolButton *btn : buttons)
    {
        btn->setAttribute(Qt::WA_TransparentForMouseEvents, ro ? true : false);
        btn->setFocusPolicy(ro ? Qt::NoFocus : Qt::StrongFocus);
    }
}


void ColourSwatchWidget::setTheme(const Colours &theme)
{
    this->theme = theme;
    for (auto it = colourButtons.constBegin(); it != colourButtons.constEnd(); ++it)
    {
        it.value()->setStyleSheet(QStringLiteral("background-color: %1;").arg(theme.colour(it.key()).name()));
    }
}

Colours ColourSwatchWidget::currentTheme() const
{
    return theme;
}

void ColourSwatchWidget::setColour()
{
    QToolButton *buttonSender = qobject_cast<QToolButton*>(sender());
    buttonSender->clearFocus();

    for (auto it = colourButtons.constBegin(); it != colourButtons.constEnd(); ++it)
    {
        if (it.value() == buttonSender)
        {
            Q3270::Colour role = it.key();

            QColor newColour = colourDialog(theme.colour(role), buttonSender);
            if (newColour.isValid())
            {
                theme.setColour(role, newColour);
                emit colourChanged(role, theme.colour(role));
            }
            break;
        }
    }
}

QColor ColourSwatchWidget::colourDialog(QColor c, QToolButton *b)
{
    const QColor dialogColour = QColorDialog::getColor(c, this, tr("Select Colour"));

    if (dialogColour.isValid() && c != dialogColour)
    {
        b->setStyleSheet(QStringLiteral("background-color: %1;").arg(dialogColour.name()));
        return dialogColour;
    }

    return QColor();
}
