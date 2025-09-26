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

#include "Session.h"

Session Session::fromActiveSettings(const ActiveSettings &active) {

    Session s;

    s.name = active.getSessionName();
    s.description = active.getDescription();

    s.hostName = active.getHostName();
    s.hostPort = active.getHostPort();
    s.hostLU   = active.getHostLU();

    s.ruler = active.getRulerState();
    s.rulerStyle = active.getRulerStyle();
    s.cursorBlink = active.getCursorBlink();
    s.cursorBlinkSpeed = active.getCursorBlinkSpeed();
    s.cursorInheritColour = active.getCursorColourInherit();
    s.screenStretch = active.getStretchScreen();
    s.backspaceStop = active.getBackspaceStop();
    s.secureConnection = active.getSecureMode();
    s.verifyCertificate = active.getVerifyCerts();
    s.font = active.getFont();
    s.terminalX = active.getTerminalX();
    s.terminalY = active.getTerminalY();
    s.terminalModel = active.getTerminalModelName();
    s.codepage = active.getCodePage();
    s.keyboardTheme = active.getKeyboardThemeName();
    s.colourTheme = active.getColourThemeName();

    return s;
}

void Session::toActiveSettings(ActiveSettings &active) const
{
    // Stop Terminal() getting lots of signals all at once
    QSignalBlocker block(active);

    active.setSessionName(name);
    active.setDescription(description);

    active.setHostAddress(hostName, hostPort, hostLU);

    active.setRulerState(ruler);
    active.setRulerStyle(rulerStyle);
    active.setCursorBlink(cursorBlink);
    active.setCursorBlinkSpeed(cursorBlinkSpeed);
    active.setCursorColourInherit(cursorInheritColour);
    active.setStretchScreen(screenStretch);
    active.setBackspaceStop(backspaceStop);
    active.setSecureMode(secureConnection);
    active.setVerifyCerts(verifyCertificate);
    active.setFont(font);
    active.setTerminal(terminalX, terminalY, terminalModel);
    active.setCodePage(codepage);
    active.setKeyboardTheme(keyboardTheme);
    active.setColourTheme(colourTheme);
}
