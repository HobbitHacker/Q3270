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
    QString functionName;
    QStringList keys;

};

struct KeyboardMap
{
    QString name;
    QList<Mapping> mappings;

    static KeyboardMap getFactoryMap();

    void set(const QString &function, const QStringList &keys);
    QStringList getFunctions() const;

    void forEach(std::function<void(const QString&, const QStringList&)> fn) const;
    void setKeyMapping(const QString &functionName, const QKeySequence &sequence);

    void dumpMaps(const QString &tag) const;

};

#endif // KEYBOARDMAP_H
