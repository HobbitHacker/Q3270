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
#include <QMap>

namespace Ui {
    class ColourTheme;
    class NewTheme;
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

        const Colours getTheme(QString theme);
        void setButtonColours(Colours theme, QHash<Colour, QPushButton *>);
        QList<QString> getThemes();

        int exec();

    private:
        Ui::ColourTheme *ui;
        Ui::NewTheme *newTheme;

        QDialog newThemePopUp;

        QHash<Colour, QPushButton *> colourButtons;
        QList<QPushButton *> extendedButtons;

        QMap<QString, Colours> themes;

        Colours colours;

        Colours currentTheme;
        QString currentThemeName;
        int currentThemeIndex;

        bool error;

        void setTheme(QString themeName);

    private slots:

        void setColour();
        void themeChanged(int index);
        void addTheme();
        void deleteTheme();
        void checkDuplicate();

        void colourDialog(QColor &c, QPushButton *b);

        void accept();

};

#endif // COLOURTHEME_H
