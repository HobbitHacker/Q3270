/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QLineEdit>

#include "KeyboardSequenceEdit.h"
#include "Q3270.h"

/**
 * @brief   KeyboardSequenceEdit::KeyboardSequenceEdit - Extended KeySequenceEdit
 * @param   parent - parent object
 *
 * @details KeyboardSequenceEdit extends the QKeySequenceEdit to handle Left-Ctrl and Right-Ctrl
 *          keys, so that Q3270 can support Reset and Enter where they were on a 3270.
 */
KeyboardSequenceEdit::KeyboardSequenceEdit(QWidget *parent)
    : QKeySequenceEdit(parent),
    ctrlUsedInChord(false)
{
}

/**
 * @brief   KeyboardSequenceEdit::keyPressEvent - react to a keypress
 * @param   event - the incoming keyboard event
 *
 * @details keyPressEvent handles the incoming keyboard event. If either Ctrl key is pressed, the
 *          UI field shows 'LCtrl..' or 'RCtrl..' instead of just 'Ctrl..'. If another key is pressed
 *          in combination with the Ctrl key (eg, CTRL-A) then the display reverts to 'Ctrl+A'.
 */
void KeyboardSequenceEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control) {
        if (event->nativeVirtualKey() == 0xffe3) {
            pendingCtrlText = "LCtrl";
        } else if (event->nativeVirtualKey() == 0xffe4) {
            pendingCtrlText = "RCtrl";
        }

        ctrlPending = true;
        clear();
        if (QLineEdit *le = findChild<QLineEdit*>()) {
            le->setText(pendingCtrlText + "..");
        }
        return;
    }

    if (ctrlPending) {
        // Ctrl was down and another key pressed: collapse to normal sequence
        ctrlPending = false;
        pendingCtrlText.clear();
        clear();
        QKeySequenceEdit::keyPressEvent(event);
        return;
    }

    QKeySequenceEdit::keyPressEvent(event);
}

/**
 * @brief   KeyboardSequenceEdit::keyReleaseEvent - react to a key release
 * @param   event - the incoming key release event
 *
 * @details keyReleaseEvent handles the left and right Ctrl keys being pressed and released
 *          in isolation; if they are pressed and released without another key being pressed,
 *          the display shows 'LCtrl' or 'RCtrl' and that sequence can be mapped.
 */
void KeyboardSequenceEdit::keyReleaseEvent(QKeyEvent *event)
{
    if (ctrlPending && event->key() == Qt::Key_Control) {
        ctrlPending = false;
        if (QLineEdit *le = findChild<QLineEdit*>()) {
            le->setText(pendingCtrlText);
        }
        emit specialKeyCaptured(pendingCtrlText);
        pendingCtrlText.clear();
        return;
    }

    QKeySequenceEdit::keyReleaseEvent(event);
}

/*
void KeyboardSequenceEdit::focusInEvent(QFocusEvent *event)
{
    QKeySequenceEdit::focusInEvent(event);

    if (QLineEdit *le = findChild<QLineEdit*>()) {
        le->setPlaceholderText(tr("Ready for input…"));
    }
}

void KeyboardSequenceEdit::focusOutEvent(QFocusEvent *event)
{
    QKeySequenceEdit::focusOutEvent(event);

    if (QLineEdit *le = findChild<QLineEdit*>()) {
        le->setPlaceholderText(tr(""));
    }
}
*/
