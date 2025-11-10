/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QUrl>
#include <QMetaEnum>
#include <QSettings>

#include "Q3270.h"
#include "SessionStore.h"

SessionStore::SessionStore()
        : settings(Q3270_ORG, Q3270_APP)
{

}

void SessionStore::load()
{
    Session s;

    QMetaEnum rs = QMetaEnum::fromType<Q3270::RulerStyle>();
    QMetaEnum ft = QMetaEnum::fromType<Q3270::FontTweak>();
    
    settings.beginGroup("Sessions"); // All Sessions

    for (QString name : settings.childGroups())
    {
        s.name = name;

        settings.beginGroup(name);      // This Session

        s.description        = settings.value("Description").toString();
        s.hostName           = settings.value("HostAddress").toString();
        s.hostPort           = settings.value("HostPort").toInt();
        s.hostLU             = settings.value("HostLU").toString();
        s.colourTheme        = settings.value("ColourTheme").toString();
        s.keyboardTheme      = settings.value("KeyboardTheme").toString();
        s.terminalModel      = settings.value("TerminalModel").toString();
        s.terminalX          = settings.value("TerminalX").toInt();
        s.terminalY          = settings.value("TerminalY").toInt();
        s.cursorBlink        = settings.value("CursorBlink").toBool();
        s.cursorBlinkSpeed   = settings.value("CursorBlinkSpeed").toInt();
        s.cursorInheritColour= settings.value("CursorInheritColour").toBool();
        s.ruler              = settings.value("Ruler").toBool();

        // Convert the RulerStyle readable form to the enum
        QByteArray styleKey = settings.value("RulerStyle").toString().toUtf8();
        s.rulerStyle = static_cast<Q3270::RulerStyle>(rs.keyToValue(styleKey));

        // Font setup
        s.font.setFamily   (settings.value("Font").toString());
        s.font.setPointSize(settings.value("FontSize").toInt());
        s.font.setStyleName(settings.value("FontStyle").toString());

        // Convert the FontTweak readable form to the enum        
        QByteArray tweakKey = settings.value("FontTweak").toString().toUtf8();
        s.tweaks            = static_cast<Q3270::FontTweak>(ft.keysToValue(tweakKey));

        s.screenStretch     = settings.value("ScreenStretch").toBool();
        s.codepage          = settings.value("Codepage").toString();
        s.secureConnection  = settings.value("SecureConnection").toBool();
        s.verifyCertificate = settings.value("VerifyCertificate").toBool();

        settings.endGroup(); // This Session

        sessions.insert(name, s);

    }

    settings.endGroup(); // All Sessions
}

Session SessionStore::getSession(const QString &name) const
{
    return sessions.value(name);
}


bool SessionStore::saveSession(const Session &session)
{
    if (session.name.trimmed().isEmpty())
        return false;

    settings.beginGroup("Sessions");

    settings.beginGroup(session.name);

    QMetaEnum rs = QMetaEnum::fromType<Q3270::RulerStyle>();
    QMetaEnum ft = QMetaEnum::fromType<Q3270::FontTweak>();

    settings.setValue("Description", session.description);
    settings.setValue("ColourTheme", session.colourTheme);
    settings.setValue("KeyboardTheme", session.keyboardTheme);
    settings.setValue("HostAddress", session.hostName);
    settings.setValue("HostPort", session.hostPort);
    settings.setValue("HostLU", session.hostLU);
    settings.setValue("TerminalModel", session.terminalModel);
    settings.setValue("TerminalX", session.terminalX);
    settings.setValue("TerminalY", session.terminalY);
    settings.setValue("CursorBlink", session.cursorBlink);
    settings.setValue("CursorBlinkSpeed", session.cursorBlinkSpeed);
    settings.setValue("CursorInheritColour", session.cursorInheritColour);
    settings.setValue("Ruler", session.ruler);
    settings.setValue("RulerStyle", QString(rs.valueToKey(session.rulerStyle)));
    settings.setValue("Font", session.font.family());
    settings.setValue("FontSize", session.font.pointSize());
    settings.setValue("FontStyle", session.font.styleName());
    settings.setValue("FontTweak", QString(ft.valueToKey(session.tweaks)));
    settings.setValue("ScreenStretch", session.screenStretch);
    settings.setValue("Codepage", session.codepage);
    settings.setValue("SecureConnection", session.secureConnection);
    settings.setValue("VerifyCertificate", session.verifyCertificate);

    settings.endGroup(); // session group
    settings.endGroup(); // Sessions group

    return true;
}

QList<Session> SessionStore::listSessions() const
{
    QList<Session> result;

    for (auto it = sessions.cbegin(); it != sessions.cend(); ++it)
    {
        result.append(it.value());
    }

    return result;
}

QStringList SessionStore::listSessionNames() const
{
    QStringList names;

    settings.beginGroup("Sessions");
    const QStringList &groups = settings.childGroups();
    settings.endGroup();

    for (const QString &encodedName : groups) {
        QString decodedName = QUrl::fromPercentEncoding(encodedName.toUtf8());
        names.append(decodedName);
    }

    return names;
}

void SessionStore::deleteSession(const QString &name)
{
    settings.beginGroup("Sessions");
    settings.remove(name); // removes the whole group
    settings.endGroup();

    settings.sync(); // ensure it’s written to disk
}

QStringList SessionStore::listAutoStartSessions() const
{
    QList<QString> realSessions = listSessionNames();

    QStringList autoStart;

    int autoCount = settings.beginReadArray("AutoStartList");

    for(int i = 0; i < autoCount; i++)
    {
        settings.setArrayIndex(i);
        const QString asName = settings.value("Session").toString();
        if (realSessions.contains(asName))
            autoStart.append(settings.value("Session").toString());
    }

    settings.endArray();

    return autoStart;
}

bool SessionStore::saveAutoStartSessions(const QStringList &list)
{
    QList<QString> realSessions = listSessionNames();

    settings.beginWriteArray("AutoStartList");

    int index = 0;

    for (const QString &asName : list)
    {
        if (realSessions.contains(asName))
        {
            settings.setArrayIndex(index);
            settings.setValue("Session", asName);
            index++;
        }
    }

    settings.endArray();

    return true;
}
