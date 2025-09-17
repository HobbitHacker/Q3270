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
