#ifndef SESSION_H
#define SESSION_H

#include <QString>
#include <QFont>
#include <QSettings>

#include "Q3270.h"
#include "ActiveSettings.h"

struct Session {
        QString name;
        QString description;
        QString hostName;
        QString hostLU;
        int hostPort;
        QString colourTheme;
        QString keyboardTheme;
        QString terminalModel;
        int terminalX;
        int terminalY;
        bool cursorBlink;
        int cursorBlinkSpeed;
        bool cursorInheritColour;
        bool ruler;
        Q3270::RulerStyle rulerStyle;
        QFont font;
        bool screenStretch;
        QString codepage;
        bool secureConnection;
        bool verifyCertificate;
        bool backspaceStop;

    static Session fromActiveSettings(const ActiveSettings &settings);
    void toActiveSettings(ActiveSettings &settings) const;
};

#endif // SESSION_H
