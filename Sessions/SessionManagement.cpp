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

#include "Q3270.h"
#include "SessionStore.h"

#include "SessionManagement.h"
#include "SaveSessionDialog.h"
#include "OpenSessionDialog.h"
#include "ManageSessionsDialog.h"

/**
 * @brief   SessionManagement::SessionManagement - All aspects of Session management
 *
 * @details This handles all aspected of Session Management. Saved Sessions contain all settings that
 *          define a given session. This class allows:
 *
 *          - Save & Load sessions
 *          - Delete session
 *          - Add/Delete to the Autostart list
 */
SessionManagement::SessionManagement(ActiveSettings &activeSettings) :
    QDialog() ,
    activeSettings(activeSettings)
{

}

/**
 * @brief   SessionManagement::~SessionManagement - destructor
 *
 * @details Delete any objects obtained via 'new'
 */
SessionManagement::~SessionManagement()
{

}

/**
 * --------------------------------------------------------------------------------------
 * Save Session Methods
 * --------------------------------------------------------------------------------------
 */

/**
 * @brief   SessionManagement::saveSessionAs - save a session with a new name
 * @return  true for OK, false for Cancel
 */
bool SessionManagement::saveSessionAs()
{
    SaveSessionDialog dlg(activeSettings, this);

    if (dlg.exec() == QDialog::Accepted) {
        // Optionally refresh session list or preview
        return true;
    }

    // If the session was previously named, return true to preserve state
    return false;
}

/**
 * --------------------------------------------------------------------------------------
 * Open Session Methods
 * --------------------------------------------------------------------------------------
 */

/**
 * @brief   SessionManagement::openSession - open an existing session
 * @param   t - the terminal
 * @return  true for OK, false for Cancel
 *
 * @details Display a list of sessions for the user to select one to open a saved session. If they
 *          select one, open and connect to it.
 */

bool SessionManagement::openSession()
{
    OpenSessionDialog dlg(activeSettings, this);

    if (dlg.exec() == QDialog::Accepted)
    {
        // Update MRU entries
        emit sessionOpened();
        return true;
    }

    return false;
}


/**
 * --------------------------------------------------------------------------------------
 * Manage Session Methods
 * --------------------------------------------------------------------------------------
 */

/**
 * @brief   SessionManagement::manageSessions - show the Manage Sessions dialog
 *
 * @details Display the dialog allowing the user to delete sessions and to manage the autostart
 *          list.
 */
void SessionManagement::manageSessions()
{
    ManageSessionsDialog dlg(activeSettings, this);
    dlg.exec();
}
