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
 * @brief   KeyboardMap::getFactoryMap - return the hard-coded keyboard map
 * @return  a KeyboardMap containing the hard-coded defaults
 *
 * @details getFactoryMap returns the hard-coded default keyboard mapping, named 'Factory'.
 */
KeyboardMap KeyboardMap::getFactoryMap()
{
    KeyboardMap km;

    km.set("Enter",      { "Enter", "RCtrl" });
    km.set("Reset",      { "LCtrl" });
    km.set("Insert",     { "Insert" });
    km.set("Delete",     { "Delete" });
    km.set("Up",         { "Up" });
    km.set("Down",       { "Down" });
    km.set("Left",       { "Left" });
    km.set("Right",      { "Right" });

    km.set("Backspace",  { "Backspace" });

    km.set("Tab",        { "Tab" });
    km.set("Backtab",    { "Backtab", "Shift+Tab", "Shift+Backtab" });

    km.set("Home",       { "Home" });
    km.set("EraseEOF",   { "End" });
    km.set("NewLine",    { "Return" });
    km.set("EndLine",    { "Ctrl+End" });

    km.set("F1",         { "F1" });
    km.set("F2",         { "F2" });
    km.set("F3",         { "F3" });
    km.set("F4",         { "F4" });
    km.set("F5",         { "F5" });
    km.set("F6",         { "F6" });
    km.set("F7",         { "F7", "PgUp" });
    km.set("F8",         { "F8", "PgDown" });
    km.set("F9",         { "F9" });
    km.set("F10",        { "F10" });
    km.set("F11",        { "F11" });
    km.set("F12",        { "F12" });

    km.set("F13",        { "Shift+F1" });
    km.set("F14",        { "Shift+F2" });
    km.set("F15",        { "Shift+F3" });
    km.set("F16",        { "Shift+F4" });
    km.set("F17",        { "Shift+F5" });
    km.set("F18",        { "Shift+F6" });
    km.set("F19",        { "Shift+F7" } );
    km.set("F20",        { "Shift+F8" });
    km.set("F21",        { "Shift+F9" });
    km.set("F22",        { "Shift+F10" });
    km.set("F23",        { "Shift+F11" });
    km.set("F24",        { "Shift+F12" });

    km.set("PA1",        { "Alt+1" });
    km.set("PA2",        { "Alt+2" });
    km.set("PA3",        { "Alt+3" });

    km.set("Attn",       { "Escape" });

    km.set("ToggleRuler", { "Ctrl+Home" });

    km.set("Clear",       { "Pause" });

    km.set("Copy",        { "Ctrl+C" });
    km.set("Paste",       { "Ctrl+V" });

    km.set("Info",        { "Ctrl+I" });
    km.set("Fields",      { "Ctrl+F" });
    km.set("DumpScreen",  { "Ctrl-D" });

    km.name = "Factory";

    return km;

}

/**
 * @brief   KeyboardMap::getFunctions - return the functions
 * @return  A list of functions used in the map
 *
 * @details getFunctions returns a list of the functions used in the keyboard map
 */
QStringList KeyboardMap::getFunctions() const
{
    QStringList list;
    list.reserve(mappings.size()); // minor optimisation

    for (const Mapping &map : mappings)
        list.append(map.functionName);

    return list;
}

/**
 * @brief   KeyboardMap::forEach - helper function to iterate over the map
 * @param   fn - a callback function to run
 *
 * @details forEach iterates over the keyboard map and calls the passed function fn
 */
void KeyboardMap::forEach(std::function<void(const QString&, const QStringList&)> fn) const {
    for (const auto &m : mappings) {
        fn(m.functionName, m.keys);
    }
}

/**
 * @brief   KeyboardMap::setKeyMapping - set a keyboard mapping
 * @param   functionName - the Q3270 function
 * @param   symbolic     - a string representing the key sequence
 *
 * @details setKeyMapping sets the supplied symbolic key sequence to the function name in the
 *          the keyboard map. The key sequence symbolic is a normalized form like 'Ctrl+A', but
 *          also handles left Ctrl and right Ctrl independently of each other. A key sequence can
 *          only map to a single Q3270 function, so any sequence passed is removed from anywhere
 *          it appears in the map first.
 */
void KeyboardMap::setKeyMapping(const QString &functionName, const QString &symbolic)
{
    qDebug() << this->name << "Setting" << symbolic << "to" << functionName;

    // Remove this key from ALL functions, including the target to avoid duplicates
    for (Mapping &m : mappings) {
        m.keys.removeAll(symbolic);
    }

    if (!functionName.isEmpty() && functionName != "Unassigned") {
        auto it = std::find_if(mappings.begin(), mappings.end(),
                               [&](const Mapping &m) { return m.functionName == functionName; });
        if (it == mappings.end()) {
            mappings.append({functionName, {symbolic}});
        } else {
            it->keys.append(symbolic);
        }
    }
}

/**
 * @brief   KeyboardMap::set - set a given set of sequences to
 * @param   function
 * @param   keys
 */
void KeyboardMap::set(const QString &function, const QStringList &keys)
{
    for (Mapping &m : mappings) {
        if (m.functionName == function) {
            m.keys = keys; // replace the whole list
            return;
        }
    }
    mappings.append({function, keys});
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
