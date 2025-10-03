/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef COLOURSWATCHWIDGET_H
#define COLOURSWATCHWIDGET_H

#include <QToolButton>

#include "Models/Colours.h"

namespace Ui
{
    class ColourSwatchWidget;
}

class ColourSwatchWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ColourSwatchWidget(QWidget *parent = nullptr);

        void setTheme(const Colours &theme);
        void setReadOnly(bool ro);
        Colours currentTheme() const;

    signals:
        void colourChanged(Q3270::Colour role, const QColor &newColour);

    private slots:
        void setColour(); // slot for swatch clicks

    private:
        QColor colourDialog(QColor c, QToolButton *b);

        Ui::ColourSwatchWidget *ui;

        QMap<Q3270::Colour, QToolButton*> colourButtons;
        Colours theme;
};

#endif // COLOURSWATCHWIDGET_H
