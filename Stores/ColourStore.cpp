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

#include "ColourStore.h"

ColourStore::ColourStore() : settings(Q3270_ORG, Q3270_APP)
{
    load();
}

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

void ColourStore::loadFactoryTheme()
{
    themes.insert("Factory", Colours::getFactoryTheme());
}

Colours ColourStore::getTheme(const QString &name) const
{

    if (name == "Factory")
        return Colours::getFactoryTheme();

    return themes.value(name, Colours::getFactoryTheme());
}

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

void ColourStore::setTheme(const QString &name, const Colours &theme)
{
    if (name == "Factory")
        return;

    themes.remove(name);
    themes.insert(name, theme);
}

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


void ColourStore::removeTheme(const QString &name)
{
    // Can't delete Factory
    if (name == "Factory")
        return;

    themes.remove(name);
}

const QStringList ColourStore::themeNames() const
{
    QStringList names = themes.keys();
    names.removeAll("Factory");
    names.prepend("Factory");

    return names;
}

bool ColourStore::exists(const QString &theme) const
{
    return themes.keys().contains(theme);
}
