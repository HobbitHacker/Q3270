#ifndef OPENSESSIONDIALOG_H
#define OPENSESSIONDIALOG_H

#include "SessionDialogBase.h"

class OpenSessionDialog : public SessionDialogBase
{
    Q_OBJECT

    public:
        explicit OpenSessionDialog(ActiveSettings &activeSettings, QWidget *parent = nullptr);

    private:

        ActiveSettings &activeSettings;

    private slots:

        void onOpenClicked();
};

#endif
