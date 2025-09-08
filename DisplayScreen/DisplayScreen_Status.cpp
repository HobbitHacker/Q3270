/*

Copyright Ⓒ 2023 Andy Styles
All Rights Reserved


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of The Qt Company Ltd nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "DisplayScreen.h"

/**
 * @brief   DisplayScreen::setStatusXSystem - set XSystem text
 * @param   text - text to be shown
 *
 * @details Called when XSystem is to be shown or removed.
 */
void DisplayScreen::setStatusXSystem(QString text)
{
    statusXSystem.setText(text);
}

/**
 * @brief   DisplayScreen::setStatusInsert - the Insert mode text
 * @param   ins - true for insert mode, false for overwrite
 *
 * @details Called to show the Insert status on the status line.
 */
void DisplayScreen::setStatusInsert(bool ins)
{
    if (ins)
    {
        statusInsert.setText("\uFF3E");
    }
    else
    {
        statusInsert.setText("");
    }
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



