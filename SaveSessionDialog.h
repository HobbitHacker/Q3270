#ifndef SAVESESSIONDIALOG_H
#define SAVESESSIONDIALOG_H

#include "SessionDialogBase.h"
#include "ActiveSettings.h"

class SaveSessionDialog : public SessionDialogBase
{
    Q_OBJECT

public:
    SaveSessionDialog(ActiveSettings &settings, QWidget *parent = nullptr);

private slots:
    void onSaveClicked();
    void saveSessionNameEdited(const QString &name);
    void doDelete(const QString &name);

private:
    ActiveSettings &activeSettings;
};

#endif // SAVESESSIONDIALOG_H
