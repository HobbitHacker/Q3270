/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef COLOURSTORE_H
#define COLOURSTORE_H

#include <QMap>
#include <QStringList>
#include <QSettings>

#include "Models/Colours.h"

class ColourStore
{
    public:

        ColourStore();

        const QStringList themeNames() const;

        Colours getTheme(const QString &name) const;

        void saveTheme(const Colours &theme);
        void removeTheme(const QString &name);

        void setTheme(const QString &name, const Colours &theme);
        void setThemes(const QMap<QString, Colours> &themes);

        void saveAllThemes() const;

        bool exists(const QString &name) const;

    private:

        QMap<QString, Colours> themes;

        mutable QSettings settings;

        void load();
        void loadFactoryTheme();

        void saveColours(const Colours &theme) const;
};

#endif // COLOURSTORE_H
