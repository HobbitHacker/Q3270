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

#include "TerminalTab.h"

namespace Ui {
    class SaveSession;
    class OpenSession;
    class ManageSessions;
}

class SessionManagement : public QDialog
{
        Q_OBJECT

    public:

        explicit SessionManagement();
        ~SessionManagement();

        // Dialogs to open and save sessions
        void openSession(TerminalTab *t);
        void saveSession(TerminalTab *t);

        // Open a named session
        void openSession(TerminalTab *t, QString sessionName);

        // Manage sessions
        void manageSessions();

   signals:

        void sessionOpened(QString session);

    private:

        Ui::SaveSession *save;
        Ui::OpenSession *load;
        Ui::ManageSessions *manage;

        QString sessionName;

        // Save session details
        void saveSettings(TerminalTab *terminal);

        // Populate QTableWidget with session details
        void populateTable(QTableWidget *table);

    private slots:

        // Triggered when session name edited; used to enable/disable OK button
        void saveSessionNameEdited(QString name);

        // Triggered when table row clicked
        void saveRowClicked(int row, int column);

        // Triggered when Open table row clicked; used to enable OK button
        void openRowClicked(int row, int column);
};

#endif // SESSIONMANAGEMENT_H
