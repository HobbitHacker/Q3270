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
        KeyboardMap theme(const QString &name) const;
        KeyboardMap factoryTheme() const;

        void setTheme(const QString &name, const KeyboardMap &map);
        void setThemes(const QMap<QString, KeyboardMap> &newThemes);

        void removeTheme(const QString &name);

        void load();
        void save() const;

    private:
        void loadFactoryTheme();
        void loadFromSettings();
        void saveToSettings() const;

        QMap<QString, KeyboardMap> themes;
        mutable QSettings settings;
};

#endif // KEYBOARDSTORE_H
