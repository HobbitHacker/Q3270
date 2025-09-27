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
