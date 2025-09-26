#ifndef COLOURS_H
#define COLOURS_H

#include <QMap>
#include <QColor>
#include "Q3270.h"

struct Colours
{

        QString name;  // theme name
        QMap<Q3270::Colour, QColor> map;

        QColor colour(Q3270::Colour role) const;

        void setColour(Q3270::Colour role, const QColor &c);

        static Colours getFactoryTheme();
};


#endif // COLOURS_H
