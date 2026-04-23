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
#include <QTextStream>
#include <QDebug>

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
    : QKeySequenceEdit(parent)
{
    if (QLineEdit *le = findChild<QLineEdit*>()) {
        le->setText("");
    }
}

/**
 * @brief   KeyboardSequenceEdit::keyPressEvent - react to a keypress
 * @param   event - the incoming keyboard event
 *
 * @details keyPressEvent handles the incoming keyboard event. If either Ctrl key is pressed, the
 *          UI field shows 'LCtrl..' or 'RCtrl..' instead of just 'Ctrl..'. If another key is pressed
 *          in combination with the Ctrl key (eg, CTRL-A) then the display reverts to 'Ctrl+A'. Shift
 *          and Alt are handled in the same way, but as they do not have left and right variants, the
 *          display just shows 'Shift..' or 'Alt..'.
 *
 *          Also if another modifier is pressed while a Ctrl key is pending, the pending state is cleared
 *          and the display reset to empty, as multiple modifiers are not supported.
 */
void KeyboardSequenceEdit::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "Key pressed - nativeVirtualKey:" << Qt::hex << event->nativeVirtualKey()
    << "nativeScanCode:" << event->nativeScanCode()
    << "nativeModifiers:" << event->nativeModifiers();

    switch (event->key()) {
        case Q3270_CTRL_KEY:
            pendingCtrlText = (event->nativeVirtualKey() == Q3270_LEFT_CTRL) ? "LCtrl" : "RCtrl";
            break;
        case Qt::Key_Alt:
            pendingCtrlText = "Alt";
            break;
        case Qt::Key_Shift:
            pendingCtrlText = "Shift";
            break;
        case Q3270_META_KEY:
            pendingCtrlText = Q3270_META_TEXT;
            break;
        default:
            break;
    }

    if (!pendingCtrlText.isEmpty() && !ctrlPending) {
        ctrlPending = true;
        QKeySequenceEdit::clear();
        if (QLineEdit *le = findChild<QLineEdit*>()) {
            le->setText(pendingCtrlText + "..");
        }
        return;
    }

    if (ctrlPending) {
        // Another modifier while one is already pending - not supported, reset
        if (event->key() == Q3270_CTRL_KEY ||
            event->key() == Qt::Key_Shift ||
            event->key() == Qt::Key_Alt) {
            ctrlPending = false;
            pendingCtrlText.clear();
            QKeySequenceEdit::clear();
            return;
        }

        // A real key arrived - finalize the chord
        ctrlPending = false;
        pendingCtrlText.clear();
        QKeySequenceEdit::clear();
        QKeySequenceEdit::keyPressEvent(event);
        if (!keySequence().isEmpty()) {
            setKeySequence(keySequence()[0]);
        }
        return;
    }

    QKeySequenceEdit::clear();
    QKeySequenceEdit::keyPressEvent(event);
    if (!keySequence().isEmpty()) {
        setKeySequence(keySequence()[0]);
    }
}

/**
 * @brief   KeyboardSequenceEdit::keyReleaseEvent - react to a key release
 * @param   event - the incoming key release event
 *
 * @details keyReleaseEvent handles the left and right Ctrl keys being pressed and released
 *          in isolation; if they are pressed and released without another key being pressed,
 *          the display shows 'LCtrl' or 'RCtrl' and that sequence can be mapped.
 *
 *          Also if Shift or Alt is released while a Ctrl key is pending, the pending state is cleared
 *          and the display reset to empty, as multiple modifiers are not supported.
 */
void KeyboardSequenceEdit::keyReleaseEvent(QKeyEvent *event)
{
    if (ctrlPending && event->key() == Q3270_CTRL_KEY) {
        ctrlPending = false;
        if (QLineEdit *le = findChild<QLineEdit*>()) {
            le->setText(pendingCtrlText);
        }
        emit specialKeyCaptured(pendingCtrlText);
        pendingCtrlText.clear();
        return;
    }

    if (ctrlPending && (event->key() == Qt::Key_Shift ||
                        event->key() == Qt::Key_Alt ||
                        event->key() == Q3270_META_KEY)) {
        // Released alone - not a valid mapping, reset silently
        ctrlPending = false;
        pendingCtrlText.clear();
        QKeySequenceEdit::clear();
        return;
    }

    QKeySequenceEdit::keyReleaseEvent(event);
}


/**
 * @brief   KeyboardSequenceEdit::focusInEvent - react to the field gaining focus
 * @param   event - the incoming focus event
 *
 * @details focusInEvent sets the placeholder text to 'Press shortcut...' when the field gains focus.
 */
void KeyboardSequenceEdit::focusInEvent(QFocusEvent *event)
{
    QKeySequenceEdit::focusInEvent(event);
    QKeySequenceEdit::clear();

    if (QLineEdit *le = findChild<QLineEdit*>()) {
        le->setPlaceholderText(tr("Press shortcut…"));
    }
}

/**
 * @brief   KeyboardSequenceEdit::focusOutEvent - react to the field losing focus
 * @param   event - the incoming focus event
 *
 * @details focusOutEvent clears the placeholder text when the field loses focus.
 */
void KeyboardSequenceEdit::focusOutEvent(QFocusEvent *event)
{
    QKeySequenceEdit::focusOutEvent(event);

    if (QLineEdit *le = findChild<QLineEdit*>()) {
        le->setPlaceholderText(tr(""));
    }
}
