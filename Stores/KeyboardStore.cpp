#include <QSettings>

#include "Q3270.h"
#include "KeyboardStore.h"

KeyboardStore::KeyboardStore()
{

}

void KeyboardStore::load()
{
    themes.clear();
    loadFactoryTheme();
    loadFromSettings();
}

void KeyboardStore::loadFactoryTheme()
{
    KeyboardMap factory = KeyboardMap::getFactoryMap();
    themes.insert("Factory", factory);
}

void KeyboardStore::loadFromSettings()
{
    QSettings s(Q3270_ORG, Q3270_APP);
    s.beginGroup("KeyboardThemes");

    QStringList themeList = s.childGroups();
    themeList.sort(Qt::CaseSensitive);

    const QStringList functionList = themes.value("Factory").getFunctions();

    for (const QString &themeName : themeList) {
        if (themeName.compare("Factory") == 0)
            continue;

        s.beginGroup(themeName);
        QStringList keys = s.childKeys();

        KeyboardMap map;
        map.name = themeName;

        for (const QString &keyName : keys) {
            QString function = s.value(keyName).toString();
            if (functionList.contains(function))
                map.set(function, { keyName });
        }

        if (!map.mappings.isEmpty())
            themes.insert(themeName, map);

        s.endGroup();
    }
    s.endGroup();
}

void KeyboardStore::save() const
{
    saveToSettings();
}

void KeyboardStore::saveToSettings() const
{
    QSettings s(Q3270_ORG, Q3270_APP);
    s.beginGroup("KeyboardThemes");
    s.remove(""); // clear existing

    for (auto it = themes.constBegin(); it != themes.constEnd(); ++it) {
        if (it.key() == "Factory")
            continue;

        s.beginGroup(it.key());
        for (const Mapping &m : it.value().mappings) {
            for (const QString &key : m.keys)
                s.setValue(key, m.functionName);
        }
        s.endGroup();
    }
    s.endGroup();
}

void KeyboardStore::setThemes(const QMap<QString, KeyboardMap> &newThemes)
{
    themes.clear();
    loadFactoryTheme();

    for (auto it = newThemes.constBegin(); it != newThemes.constEnd(); ++it)
    {
        if (it.key() == "Factory")
            continue;

        themes.insert(it.key(), it.value());
    }
}

void KeyboardStore::setTheme(const QString &theme, const KeyboardMap &map)
{
    if (theme == "Factory")
        return;

    themes.remove(theme);
    themes.insert(theme, map);
}

void KeyboardStore::removeTheme(const QString &theme)
{
    if (theme == "Factory")
        return;

    themes.remove(theme);
}

const QStringList KeyboardStore::themeNames() const
{
    return themes.keys();
}

KeyboardMap KeyboardStore::theme(const QString &theme) const
{
    return themes.value(theme);
}
