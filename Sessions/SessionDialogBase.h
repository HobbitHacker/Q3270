/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef SESSIONDIALOGBASE_H
#define SESSIONDIALOGBASE_H

#include <QDialog>
#include <QList>
#include <QCheckBox>


#include "Models/Session.h"
#include "Stores/SessionStore.h"

namespace Ui {
class SessionDialog;
}

class SessionDialogBase : public QDialog
{
    Q_OBJECT

public:

    explicit SessionDialogBase(SessionStore &store, QWidget *parent = nullptr);
    virtual ~SessionDialogBase();

signals:

    void deleteRequested(const QString &name);

protected:
    Ui::SessionDialog *ui;

    QList<Session> sessions;
    SessionStore &store;

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
