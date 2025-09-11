#ifndef SESSIONSTORE_H
#define SESSIONSTORE_H

//#include <QString>
//#include <QList>
#include <QSettings>

#include "Session.h"
#include "ActiveSettings.h"
//#include "Q3270.h"

class SessionStore {

    public:
        SessionStore();

        QList<Session> listSessions() const;
        QStringList listSessionNames() const;

        bool saveSession(const Session &session);
        Session loadSession(const QString &name) const;

    private:
        mutable QSettings settings;

        static Session fromActive(const ActiveSettings &active, const QString &name);
        static void toActive(const Session &session, ActiveSettings &active);
};

#endif // SESSIONSTORE_H
