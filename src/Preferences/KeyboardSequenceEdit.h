/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef KEYBOARDSEQUENCEEDIT_H
#define KEYBOARDSEQUENCEEDIT_H

#include <QKeySequenceEdit>
#include <QKeyEvent>

class KeyboardSequenceEdit : public QKeySequenceEdit
{
    Q_OBJECT

    public:
        explicit KeyboardSequenceEdit(QWidget *parent = nullptr);

    signals:
        void specialKeyCaptured(const QString &symbolic);

    protected:
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;
//        void focusInEvent(QFocusEvent *event) override;
//        void focusOutEvent(QFocusEvent *event) override;

    private:
        bool ctrlUsedInChord;
        bool ctrlPending;
        QString pendingCtrlText;

};

#endif // KEYBOARDSEQUENCEEDIT_H
