#ifndef SAVESESSION_H
#define SAVESESSION_H

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>

namespace Ui {
    class SaveSession;
}

class SaveSession : public QDialog
{
        Q_OBJECT

    public:
        explicit SaveSession(QWidget *parent = nullptr);
        ~SaveSession();

        static QString getSessionName(QString defaultName);

    private:
        Ui::SaveSession *ui;

        void sessionNameEdited(QString name);

};

#endif // SAVESESSION_H
