/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QFontDatabase>
#include <QSettings>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QMap>

#include "CodePage.h"
#include "ActiveSettings.h"

#include "Stores/KeyboardStore.h"
#include "Stores/ColourStore.h"

#include "KeyboardThemeDialog.h"
#include "ColourTheme.h"

namespace Ui {
    class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

    typedef QMap<QString, QStringList> KeyboardMap;

//        Q_ENUM(RulerStyle);

    public:

        explicit PreferencesDialog(CodePage &codepages, ActiveSettings &activeSettings, KeyboardStore &keyboardStore, ColourStore &colourStore, QWidget *parent = nullptr);
            ~PreferencesDialog();

        void showForm();

    signals:

        void tempFontChange(QFont f);
        void tempFontTweakChange(Q3270::FontTweak f);

        // Forwarded when keyboard themes are applied from the KeyboardThemeDialog
        void themesApplied(const QString &name);

        // Emitted when hostname field is not blank
        // TODO: Is this the right place for this?
        void connectValid(bool state);

    public slots:

        void connected();
        void disconnected();

    private slots:

        void changeFont(QFont f);
        void changeFontTweak(int index);

        void terminalModelDropDownChanged(int m);

        void colourThemeDropDownChanged(int index);
        void keyboardThemeDropDownChanged(int index);

        void populateColourThemeNames();
        void populateKeyboardThemeNames();

        void manageColourThemes();
        void manageKeyboardThemes();

        void securityChanged(int state);

    private:

        KeyboardStore &keyboardStore;
        ColourStore &colourStore;

        Ui::PreferencesDialog *ui;

        CodePage &codepages;
        ActiveSettings &activeSettings;

        QString colourThemeName;
        Colours colourTheme;

        QString keyboardThemeName;
        KeyboardMap keyboardTheme;

        // Used to populate the combobox with nice names
        QMap<QString, Q3270::RulerStyle> comboRulerStyle;
        QMap<QString, Q3270::FontTweak> comboFontTweak;

        void accept();
        void reject();
};

#endif // PREFERENCESDIALOG_H
