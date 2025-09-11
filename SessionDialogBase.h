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
    enum Mode {
        Save,
        Open,
        Manage
    };

    explicit SessionDialogBase(Mode mode, QWidget *parent = nullptr);
    virtual ~SessionDialogBase();

protected:
    Ui::SessionDialog *ui;
    Mode mode;

    QList<Session> sessions;
    SessionStore store;

    // Core logic
    void setupTable();
    void populateSessionTable();
    void connectSignals();
    void onRowClicked(int row);
    void updatePreview(const Session &s);

    void enableOKButton(bool enabled);
    void setOKButtonText(const QString &text);

    // Subclass hooks
    virtual void onAccept();

private:

};

#endif // SESSIONDIALOGBASE_H
