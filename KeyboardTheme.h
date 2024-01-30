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

#ifndef KEYBOARDTHEME_H
#define KEYBOARDTHEME_H

#include <QDialog>
#include <QObject>
#include <QMap>
#include <QSettings>
#include <QDebug>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QTableWidgetItem>

namespace Ui {
    class KeyboardTheme;
    class NewTheme;
}

class KeyboardTheme : public QDialog
{
        Q_OBJECT

    typedef QMap<QString, QStringList> KeyboardMap;

    public:

        KeyboardTheme(QWidget *parent = nullptr);

        QStringList getThemes();
        KeyboardMap getTheme(QString keyboardThemeName);
        void populateTable(QTableWidget *table, QString mapName);

        int exec();

    private:

        Ui::KeyboardTheme *ui;
        Ui::NewTheme *newTheme;

        QDialog newThemePopUp;

        QMap<QString, KeyboardMap> themes;

        QStringList functionList;

        KeyboardMap theme;
        QString currentTheme;
        int currentThemeIndex;

        int lastRow;
        int lastSeq;

        // Variables used to store state, to be restored should the user press cancel
        QString restoreTheme;
        QMap<QString, KeyboardMap> restoreThemes;
        int restoreThemeIndex;

        void setTheme(QString theme);

    private slots:

        void themeChanged(int index);
        void addTheme();
        void deleteTheme();
        void checkDuplicate();
        void populateKeySequence(QTableWidgetItem *item);
        void setKey();
        void truncateShortcut();

        void accept();
        void reject();

};

#endif // KEYBOARDTHEME_H
