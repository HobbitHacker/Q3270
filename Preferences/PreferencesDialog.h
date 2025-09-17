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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QMap>

#include "ColourTheme.h"
#include "KeyboardThemeDialog.h"
#include "CodePage.h"
#include "ActiveSettings.h"

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
        Q_OBJECT

        typedef QMap<QString, QStringList> KeyboardMap;

//        Q_ENUM(RulerStyle);

    public:

        explicit PreferencesDialog(ColourTheme &colours, CodePage &codepages, ActiveSettings &activeSettings, QWidget *parent = nullptr);
        ~PreferencesDialog();

        void showForm();

    signals:

        void tempFontChange(QFont f);

        // Emitted when hostname field is not blank
        // TODO: Is this the right place for this?
        void connectValid(bool state);

    public slots:

        void connected();
        void disconnected();

    private slots:

        void changeFont(QFont f);
        void terminalModelDropDownChanged(int m);

        void colourThemeDropDownChanged(int index);
        void keyboardThemeDropDownChanged(int index);

        void populateColourThemeNames();
        void populateKeyboardThemeNames();

        void manageColourThemes();
        void manageKeyboardThemes();

        void securityChanged(int state);

    private:

        KeyboardStore keyboards;

        Ui::PreferencesDialog *ui;


        QFontDialog *qfd;

        ColourTheme &colours;

        CodePage &codepages;
        ActiveSettings &activeSettings;

        QString colourThemeName;
        ColourTheme::Colours colourTheme;

        QString keyboardThemeName;
        KeyboardMap keyboardTheme;


        QFont qfdFont;

        QHash<Q3270::Colour, QPushButton *> colourButtons;

        // Used to populate the combobox with nice names
        QMap<QString, Q3270::RulerStyle> comboRulerStyle;

        void accept();
        void reject();

        void setButtonColours(QString themeName);

};

#endif // PREFERENCESDIALOG_H
