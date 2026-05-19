/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef KEYBOARDMAP_H
#define KEYBOARDMAP_H

#include <QString>
#include <QStringList>
#include <QKeySequence>

struct Mapping
{
    QString functionName;  // Q3270 function name (eg, "Enter", "Reset", "Copy")
    QStringList keys;      // list of symbolic key sequences that map to the function (eg, "Ctrl+A", "Shift+Tab")
};

struct KeyboardMap
{
    QString name;
    QList<Mapping> mappings;

    QStringList functionNames() const;

    static KeyboardMap factoryDefaults();

    void assignKeys(const QString &function, const QStringList &keys);
    void assignKey(const QString &functionName, const QString &sequence);
    void dumpMaps(const QString &tag) const;
};

#endif // KEYBOARDMAP_H
