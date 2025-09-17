#include <QDebug>

#include "KeyboardMap.h"

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

    km.name = "Factory";

    return km;

}

QStringList KeyboardMap::getFunctions() const
{
    QStringList list;
    list.reserve(mappings.size()); // minor optimisation

    for (const Mapping &map : mappings)
        list.append(map.functionName);

    return list;
}

void KeyboardMap::forEach(std::function<void(const QString&, const QStringList&)> fn) const {
    for (const auto &m : mappings) {
        fn(m.functionName, m.keys);
    }
}

void KeyboardMap::setKeyMapping(const QString &functionName,
                                const QKeySequence &sequence)
{
    const QString seqStr = sequence.toString();

    // Remove this sequence from any other function
    for (Mapping &m : mappings) {
        if (m.functionName != functionName) {
            m.keys.removeAll(seqStr);
        }
    }

    // Add to the target function if not "Unassigned"
    if (!functionName.isEmpty() && functionName != "Unassigned") {
        auto it = std::find_if(mappings.begin(), mappings.end(), [&](const Mapping &m)
                               { return m.functionName == functionName; });
        if (it == mappings.end()) {
            mappings.append({functionName, {seqStr}});
        } else {
            it->keys.append(seqStr);
        }
    }
}

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
