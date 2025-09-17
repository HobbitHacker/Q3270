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
