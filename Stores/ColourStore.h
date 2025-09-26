#ifndef COLOURSTORE_H
#define COLOURSTORE_H

#include <QMap>
#include <QStringList>
#include <QSettings>
#include "Models/Colours.h"

class ColourStore {
    public:
        ColourStore();

        const QStringList themeNames() const;

        Colours getTheme(const QString &name) const;

        void saveTheme(const Colours &theme);
        void removeTheme(const QString &name);
        bool exists(const QString &name) const;
        void addTheme(const Colours &theme);
        void saveAllThemes();
        void setTheme(const QString &name, const Colours &theme);
        void setThemes(const QMap<QString, Colours> &themes);

    private:

        void load();
        void loadFactoryTheme();
        void saveColours(const Colours &theme);

        QMap<QString, Colours> themes;

        mutable QSettings settings;
};



#endif // COLOURSTORE_H
