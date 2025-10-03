/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef KEYBOARDSTORE_H
#define KEYBOARDSTORE_H

#include <QMap>
#include <QStringList>
#include <QSettings>

#include "Models/KeyboardMap.h"

class KeyboardStore
{
    public:

        KeyboardStore();

        const QStringList themeNames() const;

        KeyboardMap getTheme(const QString &name) const;

        void removeTheme(const QString &name);

        void setTheme(const QString &name, const KeyboardMap &map);
        void setThemes(const QMap<QString, KeyboardMap> &newThemes);

        void saveAllThemes() const;

    private:

        QMap<QString, KeyboardMap> themes;

        mutable QSettings settings;

        void load();
        void loadFactoryTheme();
};

#endif // KEYBOARDSTORE_H
