#ifndef SESSIONDIALOGBASE_H
#define SESSIONDIALOGBASE_H

#include <QDialog>
#include <QList>
#include <QCheckBox>


#include "Session.h"
#include "SessionStore.h"

namespace Ui {
class SessionDialog;
}

class SessionDialogBase : public QDialog
{
    Q_OBJECT

public:

    explicit SessionDialogBase(QWidget *parent = nullptr);
    virtual ~SessionDialogBase();

signals:

    void deleteRequested(const QString &name);

protected:
    Ui::SessionDialog *ui;

    QList<Session> sessions;
    SessionStore store;

    // Core logic
    void setupTable();
    void populateSessionTable();
    void connectSignals();
    void onRowClicked(int row);

    void enableOKButton(bool enabled);
    void setOKButtonText(const QString &text);

    void requestDeleteSelected();

    // Subclass hooks
    virtual void onAccept();

protected slots:

    void doDelete(const QString &name);


private:

};

#endif // SESSIONDIALOGBASE_H
