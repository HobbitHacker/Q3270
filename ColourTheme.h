#ifndef COLOURTHEME_H
#define COLOURTHEME_H

#include <QDialog>
#include <QList>
#include <QVector>
#include <QSettings>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

namespace Ui {
    class ColourTheme;
}

class ColourTheme : public QDialog
{
        Q_OBJECT

    public:

        enum Colour
        {
            BLACK                      = 0,
            BLUE                       = 1,
            RED                        = 2,
            MAGENTA                    = 3,
            GREEN                      = 4,
            CYAN                       = 5,
            YELLOW                     = 6,
            NEUTRAL                    = 7,

            UNPROTECTED_NORMAL         = 32,
            PROTECTED_NORMAL           = 33,
            UNPROTECTED_INTENSIFIED    = 34,
            PROTECTED_INTENSIFIED      = 35
        };

        typedef QMap<Colour, QColor> Colours;

        explicit ColourTheme(QWidget *parent = nullptr);
        ~ColourTheme();

        const Colours getScheme(QString scheme);
        void setButtonColours(Colours scheme, QHash<Colour, QPushButton *>);
        QList<QString> getSchemes();

    private:
        Ui::ColourTheme *ui;

        QDialog newSchemePopUp;
        QLineEdit newSchemeName;
        QLabel newSchemeMessage;
        QDialogButtonBox newSchemePopUpButtons;

        QHash<Colour, QPushButton *> colourButtons;
        QList<QPushButton *> extendedButtons;

        QMap<QString, Colours> schemes;

        Colours colours;

        Colours currentScheme;
        QString currentSchemeName;
        int currentSchemeIndex;

        bool error;

        void setScheme(QString schemeName);

    private slots:

        void setColour();
        void schemeChanged(int index);
        void addScheme();
        void deleteScheme();
        void checkDuplicate();

        void colourDialog(QColor &c, QPushButton *b);

};

#endif // COLOURTHEME_H
