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
}

class SessionManagement : public QDialog
{
        Q_OBJECT

    public:

        explicit SessionManagement(TerminalTab *t);
        ~SessionManagement();

        // Dialog to save a session
        void saveSession();

    private:

        Ui::SaveSession *save;

        // Triggered when session name edited; used to enable/disable OK button
        void saveSessionNameEdited(QString name);

        // Triggered when table row clicked
        void saveRowClicked(int row, int column);

        // Save session details
        void saveSettings();


};

#endif // SESSIONMANAGEMENT_H
