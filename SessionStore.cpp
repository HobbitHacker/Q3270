#include <QUrl>
#include <QMetaEnum>
#include <QSettings>
#include <QDebug>

#include "Q3270.h"
#include "SessionStore.h"
#include <QDebug>

SessionStore::SessionStore()
    : settings(Q3270_ORG, Q3270_APP)  // Or however you initialize it
{

}

Session SessionStore::loadSession(const QString &name) const
{
    Session s;
    s.name = name;

    settings.beginGroup("Sessions"); // All Sessions

    qDebug() << settings.childGroups();

    settings.beginGroup(name);      // This Session

    qDebug() << settings.childKeys();

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

    // Enum deserialization
    QMetaEnum me = QMetaEnum::fromType<Q3270::RulerStyle>();
    QByteArray styleKey = settings.value("RulerStyle").toString().toUtf8();
    s.rulerStyle = static_cast<Q3270::RulerStyle>(me.keyToValue(styleKey));

    // Font setup
    s.font.setFamily   (settings.value("Font").toString());
    s.font.setPointSize(settings.value("FontSize").toInt());
    s.font.setStyleName(settings.value("FontStyle").toString());

    s.screenStretch     = settings.value("ScreenStretch").toBool();
    s.codepage          = settings.value("Codepage").toString();
    s.secureConnection  = settings.value("SecureConnection").toBool();
    s.verifyCertificate = settings.value("VerifyCertificate").toBool();

    settings.endGroup(); // This Session
    settings.endGroup(); // All Sessions

    return s;
}


bool SessionStore::saveSession(const Session &session)
{
    if (session.name.trimmed().isEmpty())
        return false;

    settings.beginGroup("Sessions");

    settings.beginGroup(session.name);

    QMetaEnum metaEnum = QMetaEnum::fromType<Q3270::RulerStyle>();

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
    settings.setValue("RulerStyle", QString(metaEnum.valueToKey(session.rulerStyle)));
    settings.setValue("Font", session.font.family());
    settings.setValue("FontSize", session.font.pointSize());
    settings.setValue("FontStyle", session.font.styleName());
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

    settings.beginGroup("Sessions");

    const QStringList sessionGroups = settings.childGroups();  // e.g. Turnkey%20MVS%203.8j%2000C0

    for (const QString &encodedName : sessionGroups) {

        // TODO: Do we need fromPercentEncoding here
        QString decodedName = QUrl::fromPercentEncoding(encodedName.toUtf8());

        settings.beginGroup(encodedName);

        Session s;

        s.name = decodedName;

        s.hostName = settings.value("HostAddress").toString();
        s.hostPort = settings.value("HostPort").toInt();
        s.hostLU   = settings.value("HostLU").toString();

        s.description = settings.value("Description").toString();
        // Add more fields here if needed

        settings.endGroup();  // Leave session subgroup
        result.append(s);
    }

    settings.endGroup();  // Leave "Sessions" group

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
