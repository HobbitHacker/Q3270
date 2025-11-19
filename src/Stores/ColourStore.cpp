/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "ColourStore.h"

/**
 * @brief   ColourStore::ColourStore constructor.
 *
 * @details This class manages the storage and retrieval of colour themes.
 */
ColourStore::ColourStore() : settings(Q3270_ORG, Q3270_APP)
{
    load();
}

/**
 * @brief   Load colour themes from persistent storage.
 *
 * @details This function reads colour themes from QSettings and populates
 *          the internal themes map. The "Factory" theme is always loaded
 *          as a default theme.
 */
void ColourStore::load()
{

    themes.clear();
    loadFactoryTheme();

    settings.beginGroup("ColourThemes");

    for (const QString &themeName : settings.childGroups())
    {
        // Skip loading Factory from disk
        if (themeName == "Factory")
            continue;

        settings.beginGroup(themeName);

        Colours c;

        c.name = themeName;

        c.map[Q3270::UnprotectedNormal]      = QColor(settings.value("UnprotectedNormal").toString());
        c.map[Q3270::UnprotectedIntensified] = QColor(settings.value("UnprotectedIntensified").toString());
        c.map[Q3270::ProtectedNormal]        = QColor(settings.value("ProtectedNormal").toString());
        c.map[Q3270::ProtectedIntensified]   = QColor(settings.value("ProtectedIntensified").toString());

        c.map[Q3270::Black]                  = QColor(settings.value("Black").toString());
        c.map[Q3270::Blue]                   = QColor(settings.value("Blue").toString());
        c.map[Q3270::Red]                    = QColor(settings.value("Red").toString());
        c.map[Q3270::Magenta]                = QColor(settings.value("Magenta").toString());
        c.map[Q3270::Green]                  = QColor(settings.value("Green").toString());
        c.map[Q3270::Cyan]                   = QColor(settings.value("Cyan").toString());
        c.map[Q3270::Yellow]                 = QColor(settings.value("Yellow").toString());
        c.map[Q3270::Neutral]                = QColor(settings.value("Neutral").toString());

        settings.endGroup();

        themes.insert(themeName, c);
    }

    settings.endGroup();
}

/**
 * @brief   Load the factory default theme.
 *
 * @details This function inserts the built-in "Factory" theme into
 *          the themes map.
 */
void ColourStore::loadFactoryTheme()
{
    themes.insert("Factory", Colours::getFactoryTheme());
}

/**
 * @brief   Retrieve a colour theme by name.
 * @param   name    The name of the theme to retrieve.
 * @return  The Colours object representing the requested theme.
 *
 * @details If the requested theme does not exist, the factory default
 *          theme is returned.
 */
Colours ColourStore::getTheme(const QString &name) const
{

    if (name == "Factory")
        return Colours::getFactoryTheme();

    return themes.value(name, Colours::getFactoryTheme());
}

/**
 * @brief   Save a colour theme.
 * @param   theme   The Colours object representing the theme to save.
 *
 * @details This function saves the given theme to persistent storage.
 *          The "Factory" theme is not saved as it is built-in.
 */
void ColourStore::saveTheme(const Colours &theme)
{
    // Don't overwrite Factory
    if (theme.name == "Factory")
        return;

    themes[theme.name] = theme;

    settings.beginGroup("ColourThemes");
    saveColours(theme);
    settings.endGroup(); // ColourThemes
}

/**
 * @brief   Save all colour themes to persistent storage.
 *
 * @details This function iterates over all stored themes and saves
 *          them to QSettings, excluding the "Factory" theme.
 */
void ColourStore::saveAllThemes() const
{
    settings.beginGroup("ColourThemes");
    settings.remove("");

    for (const Colours &theme : themes)
    {
        saveColours(theme);
    }

    settings.endGroup();

}

/**
 * @brief   Save a single colour theme to persistent storage.
 * @param   theme   The Colours object representing the theme to save.
 *
 * @details This function writes the colours of the given theme
 *          into QSettings under its respective group.
 */
void ColourStore::saveColours(const Colours &theme) const
{
    if (theme.name == "Factory")
        return;

    settings.beginGroup(theme.name);

    // Save base colours
    settings.setValue("UnprotectedNormal",      theme.map[Q3270::UnprotectedNormal].name());
    settings.setValue("UnprotectedIntensified", theme.map[Q3270::UnprotectedIntensified].name());
    settings.setValue("ProtectedNormal",        theme.map[Q3270::ProtectedNormal].name());
    settings.setValue("ProtectedIntensified",   theme.map[Q3270::ProtectedIntensified].name());

    // Save Extended colours
    settings.setValue("Black",                  theme.map[Q3270::Black].name());
    settings.setValue("Blue",                   theme.map[Q3270::Blue].name());
    settings.setValue("Red",                    theme.map[Q3270::Red].name());
    settings.setValue("Magenta",                theme.map[Q3270::Magenta].name());
    settings.setValue("Green",                  theme.map[Q3270::Green].name());
    settings.setValue("Cyan",                   theme.map[Q3270::Cyan].name());
    settings.setValue("Yellow",                 theme.map[Q3270::Yellow].name());
    settings.setValue("Neutral",                theme.map[Q3270::Neutral].name());

    settings.endGroup();
}

/**
 * @brief   Set or update a colour theme.
 * @param   name    The name of the theme to set.
 * @param   theme   The Colours object representing the theme.
 *
 * @details This function adds or updates the specified theme
 *          in the internal themes map. The "Factory" theme
 *          cannot be modified.
 */
void ColourStore::setTheme(const QString &name, const Colours &theme)
{
    if (name == "Factory")
        return;

    themes.remove(name);
    themes.insert(name, theme);
}

/**
 * @brief   Set multiple colour themes at once.
 * @param   newThemes   A map of theme names to Colours objects.
 *
 * @details This function replaces the current themes with the
 *          provided set, ensuring the "Factory" theme remains
 *          intact.
 */
void ColourStore::setThemes(const QMap<QString, Colours> &newThemes)
{
    themes.clear();
    loadFactoryTheme();

    for (auto it = newThemes.cbegin(); it != newThemes.cend(); it++)
    {
        if (it.key() == "Factory")
            continue;

        themes.insert(it.key(), it.value());
    }
}

/**
 * @brief   Remove a colour theme by name.
 * @param   name    The name of the theme to remove.
 *
 * @details This function deletes the specified theme from
 *          the internal themes map. The "Factory" theme
 *          cannot be removed.
 */
void ColourStore::removeTheme(const QString &name)
{
    // Can't delete Factory
    if (name == "Factory")
        return;

    themes.remove(name);
}

/**
 * @brief   Retrieve the list of available theme names.
 * @return  A QStringList containing the names of all themes.
 *
 * @details The "Factory" theme is always listed first.
 */
const QStringList ColourStore::themeNames() const
{
    QStringList names = themes.keys();
    names.removeAll("Factory");
    names.prepend("Factory");

    return names;
}

/**
 * @brief   Check if a colour theme exists.
 * @param   name    The name of the theme to check.
 * @return  True if the theme exists, false otherwise.
 */
bool ColourStore::exists(const QString &theme) const
{
    return themes.keys().contains(theme);
}
