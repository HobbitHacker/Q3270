#ifndef MANAGESESSIONSDIALOG_H
#define MANAGESESSIONSDIALOG_H

#include "SessionDialogBase.h"

class ManageSessionsDialog : public SessionDialogBase
{
    Q_OBJECT
public:
    explicit ManageSessionsDialog(ActiveSettings &activeSettings, QWidget *parent = nullptr);

private slots:

    void onManageAutoStartClicked();

private:
    QPushButton *autoStartButton;
};

#endif // MANAGESESSIONSDIALOG_H
