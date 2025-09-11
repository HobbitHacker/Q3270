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

#ifndef SESSIONMANAGEMENT_H
#define SESSIONMANAGEMENT_H

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>
#include <QTableWidget>

#include "Terminal.h"
#include "PreferencesDialog.h"
#include "ActiveSettings.h"
#include "SessionStore.h"

namespace Ui {
    class SaveSession;
    class OpenSession;
    class ManageSessions;
    class AutoStart;
    class AutoStartAdd;
}

class SessionManagement : public QDialog
{
        Q_OBJECT

    public:

        explicit SessionManagement(ActiveSettings &activeSettings);
        ~SessionManagement();

        // Dialogs to open and save sessions; return true if session opened/saved
//        void openSession(Terminal *t, const QString &sessionName);
        bool openSession();
        bool saveSessionAs();

        // Manage sessions
        void manageSessions();

   signals:

        void sessionOpened();
        void autoStartAddToList(int row);

   private:

        Ui::SaveSession *save;
        Ui::OpenSession *load;
        Ui::ManageSessions *manage;
        Ui::AutoStart *autostart;
        Ui::AutoStartAdd *add;

        // Session name / description once session opened
        QString sessionName;
        QString sessionDesc;

        ActiveSettings &activeSettings;

        SessionStore store;

        // Populate QTableWidget with session details
        void populateTable(QTableWidget *table);

    private slots:

        // Manage Autostart list - button on Manage Sessions dialog
        void manageAutoStartList();

        // Delete session button
        void deleteSession();

        // Triggered when Manage Sessions table row clicked; used to enable Delete button
        void manageRowClicked(int row, int column);

        // Triggered when Autostart list row clicked; used to enable Delete button
        void autoStartCellClicked(int row, int column);

        // Autostart Add button
        void addAutoStart();

        // Autostart Delete button
        void deleteAutoStart();

        // Triggered when Autostart Add list row clicked; used to enable OK button
        void autoStartAddCellClicked(int row, int col);

        // Emitted when a row is added to the autostart list
        void autoStartRowAdded(int row);

};

#endif // SESSIONMANAGEMENT_H
