/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef COLOURTHEME_H
#define COLOURTHEME_H

#include <QDialog>
#include <QList>
#include <QVector>
#include <QSettings>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMap>

#include "Q3270.h"
#include "Stores/ColourStore.h"

namespace Ui {
    class ColourTheme;
}

class ColourTheme : public QDialog
{
        Q_OBJECT

    public:

        explicit ColourTheme(ColourStore &store, QWidget *parent = nullptr);
        ~ColourTheme();

//        void setButtonColours(Colours theme, QString themeName);
//        void setButtonColours(QString themeName);
//        QList<QString> getThemes();

        int exec() override;

        void setTheme(const QString &themeName);

    private:

        ColourStore &store;

        Ui::ColourTheme *ui;

        QMap<QString, Colours>themes;
        Colours *currentTheme;

        // Variables used to restore state, should the user presss cancel
        QMap<QString, Colours>restoreThemes;
        QString restoreThemeName;

        bool dirty;
        bool unapplied;

        void updateUiState();

    signals:

        void themesApplied(const QString &name);

    private slots:

        void handleThemeChanged(const QString &name);
        void handleColourModified(Q3270::Colour, QColor);

        void checkThemeName(const QString &name);

        void createNewTheme();
        void saveTheme();
        void removeTheme();

        void revertTheme();
        void applyTheme();
};

#endif // COLOURTHEME_H
