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
#ifndef MANAGEAUTOSTARTDIALOG_H
#define MANAGEAUTOSTARTDIALOG_H

#include <QDialog>
#include <QListWidget>

#include "Stores/SessionStore.h"

namespace Ui {
class ManageAutoStartDialog;
}

class ManageAutoStartDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ManageAutoStartDialog(SessionStore &store, QWidget *parent = nullptr);
    ~ManageAutoStartDialog();

private slots:
    void handleAddButtonClicked();
    void handleRemoveButtonClicked();

    void onAvailableRowClicked(QListWidgetItem *item);
    void onAutoStartRowClicked(QListWidgetItem *item);

    void onAccept();

private:
    void refreshLists();
    void populateList(QListWidget *listWidget, const QList<Session> &sessions);

    Ui::ManageAutoStartDialog *ui;
    SessionStore &store;

    QList<Session> allSessions;
    QStringList autoStartSessions;
};

#endif // MANAGEAUTOSTARTDIALOG_H
