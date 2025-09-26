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

#include "Q3270.h"
#include "Stores/ColourStore.h"

namespace Ui {
    class ColourTheme;
}

class ColourTheme : public QDialog
{
        Q_OBJECT

    public:

        explicit ColourTheme(ColourStore &store, QWidget *parent = nullptr);
        ~ColourTheme();

//        void setButtonColours(Colours theme, QString themeName);
//        void setButtonColours(QString themeName);
//        QList<QString> getThemes();

        int exec() override;

        void setTheme(const QString &themeName);

    private:

        ColourStore &store;

        Ui::ColourTheme *ui;

        QMap<QString, Colours>themes;
        Colours *currentTheme;

        // Variables used to restore state, should the user presss cancel
        QMap<QString, Colours>restoreThemes;
        QString restoreThemeName;

        bool dirty;
        bool unapplied;

        void updateUiState();

    signals:

        void themesApplied(const QString &name);

    private slots:

        void handleThemeChanged(const QString &name);
        void handleColourModified(Q3270::Colour, QColor);

        void checkThemeName(const QString &name);

        void createNewTheme();
        void saveTheme();
        void removeTheme();

        void revertTheme();
        void applyTheme();
};

#endif // COLOURTHEME_H
