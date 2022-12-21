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
#include "PreferencesDialog.h"
#include "DisplayScreen.h"
#include "ActiveSettings.h"

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

        explicit SessionManagement(PreferencesDialog *settings, ActiveSettings *activeSettings);
        ~SessionManagement();

        // Dialogs to open and save sessions; return true if session opened/saved
        bool openSession(TerminalTab *t);
        bool saveSessionAs(TerminalTab *t);

        // Save current named session details
        void saveSettings();

        // Open a named session
        void openSession(TerminalTab *t, QString sessionName);

        // Manage sessions
        void manageSessions();

   signals:

        void sessionOpened(QString session);
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

        PreferencesDialog *settings;
        ActiveSettings *activeSettings;

        // Populate QTableWidget with session details
        void populateTable(QTableWidget *table);

    private slots:

        // Manage Autostart list - button on Manage Sessions dialog
        void manageAutoStartList();

        // Triggered when session name edited; used to enable/disable OK button
        void saveSessionNameEdited(QString name);

        // Triggered when table row clicked
        void saveRowClicked(int row, int column);

        // Triggered when Open table row clicked; used to enable OK button
        void openRowClicked(int row, int column);

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
