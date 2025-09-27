/*

Copyright Ⓒ 2025 Andy Styles
All Rights Reserved

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "Q3270.h"
#include "KeyboardStore.h"
/**
 * @brief   KeyboardStore::KeyboardStore - persistence layer for KeyboardMaps
 *
 * @details KeybaordStore is the persistence layer for keyboard maps. Everything is held in memory
 *          and modified there until the user actively saves changes. There is only one KeyboardStore across
 *          the entire application.
 */
KeyboardStore::KeyboardStore() : settings(Q3270_ORG, Q3270_APP)
{
    load();
}

/**
 * @brief   KeyboardStore::load - Initialise the KeyboardStore
 *
 * @details Clear the existing store, redefine the Factory map, and load any others from the config file.
 *          If Factory is present in the config file, it's ignored.
 */
void KeyboardStore::load()
{
    themes.clear();
    themes.insert("Factory", KeyboardMap::getFactoryMap());

    settings.beginGroup("KeyboardThemes");

    QStringList themeList = settings.childGroups();
    themeList.sort(Qt::CaseSensitive);

    const QStringList functionList = themes.value("Factory").getFunctions();

    for (const QString &themeName : themeList) {
        if (themeName == "Factory")
            continue;

        settings.beginGroup(themeName);
        QStringList keys = settings.childKeys();

        KeyboardMap map;
        map.name = themeName;

        for (const QString &keyName : keys) {
            QString function = settings.value(keyName).toString();
            if (functionList.contains(function))
                map.set(function, { keyName });
        }

        if (!map.mappings.isEmpty())
            themes.insert(themeName, map);

        settings.endGroup();  // This theme
    }
    settings.endGroup(); // KeyboardThemes
}

/**
 * @brief   KeyboardStore::loadFactoryTheme - load the Factory theme into the store
 *
 * @details Populate the themes store with the Factory theme.
 */
void KeyboardStore::loadFactoryTheme()
{
    themes.insert("Factory", KeyboardMap::getFactoryMap());
}

/**
 * @brief   KeyboardStore::saveToSettings
 *
 * @details Persist the in-memory KeyboardMaps to disk in the config file.
 */
void KeyboardStore::saveAllThemes() const
{
    settings.beginGroup("KeyboardThemes");
    settings.remove(""); // clear existing

    for (auto it = themes.constBegin(); it != themes.constEnd(); ++it) {
        if (it.key() == "Factory")
            continue;

        settings.beginGroup(it.key());
        for (const Mapping &m : it.value().mappings) {
            for (const QString &key : m.keys)
                settings.setValue(key, m.functionName);
        }
        settings.endGroup();
    }
    settings.endGroup();
}

/**
 * @brief   KeyboardStore::setThemes - replace the keyboard maps in the store
 * @param   newThemes - the new keyboard maps
 *
 * @details Used by the KeyboardThemeDialog after a user has pressed 'OK'; this is used to re-populate
 *          the store with the edited KeyboardMaps which may have been changed during the dialog operation.
 *
 *          As always, the Factory map is ignore if it's present (which it should be) in the new theme list.
 */
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

/**
 * @brief   KeyboardStore::setTheme - insert/update a single keyboard map
 * @param   theme - the KeyboardMap name
 * @param   map   - the KeyboardMap
 *
 * @details Used, for example, when the user modifies a KeyboardMap and presses 'Apply'. If one exists already it
 *          is removed first, and then re-added.
 *
 *          Factory map is ignored - it can't be replaced or updated.
 */
void KeyboardStore::setTheme(const QString &theme, const KeyboardMap &map)
{
    if (theme == "Factory")
        return;

    themes.remove(theme);
    themes.insert(theme, map);
}

/**
 * @brief   KeyboardStore::removeTheme - remove a named KeyboardMap
 * @param   theme - KeyboardMap to remove
 *
 * @details The KeyboardMap specified is removed from the store.
 */
void KeyboardStore::removeTheme(const QString &theme)
{
    if (theme == "Factory")
        return;

    themes.remove(theme);
}

/**
 * @brief   KeyboardStore::themeNames - return a list of all KeyboardMap names
 * @return  A list of KeyboardMap names, "Factory" always first.
 */
const QStringList KeyboardStore::themeNames() const
{
    QStringList names;

    names = themes.keys();
    names.removeAll("Factory");
    names.prepend("Factory");

    return names;
}

/**
 * @brief   KeyboardStore::getTheme - return a named KeyboardMap
 * @param   theme - the name of the KeyboardMap
 * @return  The specified KeyboardMap
 *
 * @details Returns the named keyboard map from the store.
 *          TODO: No handling of a non-match
 */
KeyboardMap KeyboardStore::getTheme(const QString &theme) const
{
    return themes.value(theme);
}
