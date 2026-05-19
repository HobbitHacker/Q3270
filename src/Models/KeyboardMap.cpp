/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QDebug>

#include "KeyboardMap.h"

/**
 * @brief   KeyboardMap::factoryDefaults - return the hard-coded keyboard map
 * @return  a KeyboardMap containing the hard-coded defaults
 *
 * @details Returns the hard-coded default keyboard mapping, named 'Factory'.
 */
KeyboardMap KeyboardMap::factoryDefaults()
{
    KeyboardMap km;

    km.assignKeys("Enter",      { "Enter", "RCtrl" });
    km.assignKeys("Reset",      { "LCtrl" });
    km.assignKeys("Insert",     { "Insert" });
    km.assignKeys("Delete",     { "Delete" });
    km.assignKeys("Up",         { "Up" });
    km.assignKeys("Down",       { "Down" });
    km.assignKeys("Left",       { "Left" });
    km.assignKeys("Right",      { "Right" });

    km.assignKeys("Backspace",  { "Backspace" });

    km.assignKeys("Tab",        { "Tab" });
    km.assignKeys("Backtab",    { "Backtab", "Shift+Tab", "Shift+Backtab" });

    km.assignKeys("Home",       { "Home" });
    km.assignKeys("EraseEOF",   { "End" });
    km.assignKeys("NewLine",    { "Return" });
    km.assignKeys("EndLine",    { "Ctrl+End" });

    km.assignKeys("F1",         { "F1" });
    km.assignKeys("F2",         { "F2" });
    km.assignKeys("F3",         { "F3" });
    km.assignKeys("F4",         { "F4" });
    km.assignKeys("F5",         { "F5" });
    km.assignKeys("F6",         { "F6" });
    km.assignKeys("F7",         { "F7", "PgUp" });
    km.assignKeys("F8",         { "F8", "PgDown" });
    km.assignKeys("F9",         { "F9" });
    km.assignKeys("F10",        { "F10" });
    km.assignKeys("F11",        { "F11" });
    km.assignKeys("F12",        { "F12" });

    km.assignKeys("F13",        { "Shift+F1" });
    km.assignKeys("F14",        { "Shift+F2" });
    km.assignKeys("F15",        { "Shift+F3" });
    km.assignKeys("F16",        { "Shift+F4" });
    km.assignKeys("F17",        { "Shift+F5" });
    km.assignKeys("F18",        { "Shift+F6" });
    km.assignKeys("F19",        { "Shift+F7" } );
    km.assignKeys("F20",        { "Shift+F8" });
    km.assignKeys("F21",        { "Shift+F9" });
    km.assignKeys("F22",        { "Shift+F10" });
    km.assignKeys("F23",        { "Shift+F11" });
    km.assignKeys("F24",        { "Shift+F12" });

    km.assignKeys("PA1",        { "Alt+1" });
    km.assignKeys("PA2",        { "Alt+2" });
    km.assignKeys("PA3",        { "Alt+3" });

    km.assignKeys("Attn",       { "Escape" });

    km.assignKeys("ToggleRuler", { "Ctrl+Home" });

    km.assignKeys("Clear",       { "Pause" });

    km.assignKeys("Copy",        { "Ctrl+C" });
    km.assignKeys("Paste",       { "Ctrl+V" });

    km.assignKeys("Info",        { "Ctrl+I" });
    km.assignKeys("Fields",      { "Ctrl+F" });
    km.assignKeys("DumpScreen",  { "Ctrl-D" });

    km.name = "Factory";

    return km;

}

/**
 * @brief   KeyboardMap::getFunctions - return the functions
 * @return  A list of functions used in the map
 *
 * @details getFunctions returns a list of the functions used in the keyboard map
 */
QStringList KeyboardMap::functionNames() const
{
    QStringList list;
    list.reserve(mappings.size()); // minor optimisation

    for (const Mapping &map : mappings)
        list.append(map.functionName);

    return list;
}

/**
 * @brief   KeyboardMap::assignKey - set a keyboard mapping
 * @param   functionName - the Q3270 function
 * @param   symbolic     - a string representing the key sequence
 *
 * @details assignKey is a helper to assign a single key sequence for a given function.
 *          The key sequence is a normalized form like 'Ctrl+A'.
 */
void KeyboardMap::assignKey(const QString &functionName, const QString &symbolic)
{
    assignKeys(functionName, {symbolic});
}

/**
 * @brief   KeyboardMap::assignKeys - assignKeys a given assignKeys of sequences to a function
 * @param   function - the Q3270 function
 * @param   keys - a list of symbolic key sequences to map to the function
 *
 * @details assignKeys assigns a given list of key sequences to a function. The existing mapping
 *          for those keys is removed first, and then the new mapping is added.
 */
void KeyboardMap::assignKeys(const QString &functionName, const QStringList &keys)
{
    for (Mapping &m : mappings)
        for (const QString &key : keys)
            m.keys.removeAll(key);

    // Lambda for finding the mapping for the function name, if it exists
    auto matchesFunction = [&](const Mapping &m) {
        return m.functionName == functionName;
    };

    if (!functionName.isEmpty() && functionName != "Unassigned")
    {
        auto it = std::find_if(mappings.begin(), mappings.end(), matchesFunction);

        if (it == mappings.end())
            mappings.append({functionName, keys});
        else
            for (const QString &key : keys)
                if (!it->keys.contains(key))
                    it->keys.append(key);
    }
}

void KeyboardMap::dumpMaps(const QString &tag) const
{
    qDebug().noquote() << "=== KeyboardMap dump:" << (tag.isEmpty() ? tag : "") << "===";
    qDebug().noquote() << "Map name:" << name;
    for (const auto &m : mappings) {
        qDebug().noquote() << "Function:" << m.functionName
                           << " Keys:" << m.keys.join(", ");
    }
    qDebug().noquote() << "=== End dump ===";
}
