#include <QPushButton>
#include <QDialogButtonBox>


#include "ManageSessionsDialog.h"
#include "ManageAutoStartDialog.h"
#include "Stores/SessionStore.h"
//#include "ManageAutoStartDialog.h"
#include "ui_SessionDialog.h"

ManageSessionsDialog::ManageSessionsDialog(ActiveSettings &activeSettings, QWidget *parent)
    : SessionDialogBase(parent)
{
    setWindowTitle(tr("Manage Sessions"));

    // Add the "Manage AutoStart..." button alongside the base class buttons
    autoStartButton = new QPushButton(tr("Manage AutoStart..."), this);
    ui->buttonBox->addButton(autoStartButton, QDialogButtonBox::ActionRole);

    connect(autoStartButton, &QPushButton::clicked, this, &ManageSessionsDialog::onManageAutoStartClicked);
    connect(this, &SessionDialogBase::deleteRequested, this,&ManageSessionsDialog::doDelete);
}

void ManageSessionsDialog::onManageAutoStartClicked()
{
    ManageAutoStartDialog dlg(store, this);
    dlg.exec();
}
