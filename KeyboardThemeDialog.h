/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef KEYBOARDTHEMEDIALOG_H
#define KEYBOARDTHEMEDIALOG_H

#include <QDialog>
#include <QObject>
#include <QMap>
#include <QSettings>
#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTableWidgetItem>

#include "Models/KeyboardMap.h"
#include "Stores/KeyboardStore.h"

namespace Ui {
    class KeyboardTheme;
}

class KeyboardThemeDialog : public QDialog
{
        Q_OBJECT

    public:

        explicit KeyboardThemeDialog(KeyboardStore &store, QWidget *parent = nullptr);
        ~KeyboardThemeDialog();

        int exec();

    private:

        KeyboardMap factory;
        KeyboardStore &store;

        Ui::KeyboardTheme *ui;

        QMap<QString, KeyboardMap> themes;

        QStringList functionList;

        KeyboardMap theme;
        KeyboardMap *currentTheme;

        int lastRow;
        int lastSeq;

        bool dirty;
        bool unapplied;

        // Variables used to store state, to be restored should the user press cancel
        QString restoreThemeName;
        QMap<QString, KeyboardMap> restoreThemes;
        int restoreThemeIndex;

        void setTheme(const QString &themeName);
        void updateUiState();

  signals:
        void themesApplied(const QString &themeName);

  private slots:

        void handleThemeChanged(const QString &name);

        void validateThemeName(const QString &name);

        void createNewTheme();
        void saveTheme();
        void removeTheme();

//        void themeChanged(int index);
//        void addTheme();

        void setKey();
        void truncateShortcut();
        void populateKeySequence(int row, const QString &functionName, const QStringList &keyList);

        void revertTheme();
        void applyTheme();
};

#endif // KEYBOARDTHEMEDIALOG_H
