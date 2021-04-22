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

        enum ExtendedColours
        {
            BLACK                      = 0,
            BLUE                       = 1,
            RED                        = 2,
            MAGENTA                    = 3,
            GREEN                      = 4,
            CYAN                       = 5,
            YELLOW                     = 6,
            NEUTRAL                    = 7
        };

        enum BaseColours
        {
            PROTECTED_NORMAL           = 0,
            UNPROTECTED_INTENSIFIED    = 1,
            UNPROTECTED_NORMAL         = 2,
            PROTECTED_INTENSIFIED      = 3
        };

        struct Colours
        {
                QVector<QColor> base;
                QVector<QColor> extended;
        };

        explicit ColourTheme(QWidget *parent = nullptr);
        ~ColourTheme();

    private:
        Ui::ColourTheme *ui;

        QDialog newSchemePopUp;
        QLineEdit newSchemeName;
        QLabel newSchemeMessage;
        QDialogButtonBox newSchemePopUpButtons;

        QMap<QString, Colours> schemes;

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
