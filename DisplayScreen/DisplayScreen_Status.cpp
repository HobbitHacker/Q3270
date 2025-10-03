/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "DisplayScreen.h"
#include <QDateTime>

/**
 * @brief   DisplayScreen::setStatusXSystem - set XSystem text
 * @param   status - Q3270::Unlocked, Q3270::SystemLock or Q3270::TerminalWait
 *
 * @details Called when XSystem / X<clock> is to be shown or removed.
 */
void DisplayScreen::setStatusLock(Q3270::Indicators status)
{
    // Hide both first
//    statusXClock->hide();
//    statusXSystem.hide();

    switch(status)
    {
        case Q3270::TerminalWait:
            qDebug() << QDateTime::currentMSecsSinceEpoch() << "DisplayScreen   : TerminalWait - X <clock>";
//            statusXClock->show();
            statusX->setMode(LockIndicator::Clock);
            break;
        case Q3270::SystemLock:
            qDebug() << QDateTime::currentMSecsSinceEpoch() << "DisplayScreen   : TerminalWait - X System";
            statusX->setMode(LockIndicator::System);
//            statusXSystem.show();
            break;
        case Q3270::Unlocked:
            statusX->setMode(LockIndicator::None);
            qDebug() << QDateTime::currentMSecsSinceEpoch() << "DisplayScreen   : Unlocked";
    }
}

/**
 * @brief   DisplayScreen::setStatusInsert - the Insert mode text
 * @param   ins - Q3270::InsertMode or Q3270::OvertypeMode
 *
 * @details Called to show the Insert status on the status line.
 */
void DisplayScreen::setStatusInsert(Q3270::Indicators insert)
{
    statusInsert.setText(insert == Q3270::InsertMode ? QString("\uFF3E") : QString(""));
}

/**
 * @brief  DisplayScreen::setEncrypted - change encryption status bar icon
 * @param  encrypted - the encryption state
 *
 *        encrypted  | Description
 *        ---------- | -----------
 *          0        | Unencrypted
 *          1        | Encrypted without validating certs
 *          2        | Encrypted
 */
void DisplayScreen::setEncrypted(Q3270::Encryption encrypted)
{
    lock->hide();
    unlock->hide();
    locktick->hide();

    switch(encrypted)
    {
        case Q3270::Unencrypted:
            unlock->show();
            break;
        case Q3270::SemiEncrypted:
            lock->show();
            break;
        case Q3270::Encrypted:
            locktick->show();
            break;
    }

//    QRectF r = statusSecureSVG->boundingRect();
//    statusSecureSVG->setScale(8 / r.height());
}



