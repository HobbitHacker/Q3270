/*

Copyright Ⓒ 2023 Andy Styles
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
//        void setButtonColours(Colours theme, QString themeName);
        void setButtonColours(QString themeName);
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

        // Variables used to restore state, should the user presss cancel
        QMap<QString, Colours> restoreThemes;
        QString restoreThemeName;
        int restoreThemeIndex;

        void setTheme(QString themeName);

    private slots:

        void setColour();
        void themeChanged(int index);
        void addTheme();
        void deleteTheme();
        void checkDuplicate();

        void colourDialog(QColor &c, QPushButton *b);

        void accept();
        void reject();

};

#endif // COLOURTHEME_H
