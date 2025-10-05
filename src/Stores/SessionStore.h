/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef SESSIONSTORE_H
#define SESSIONSTORE_H

#include <QSettings>

#include "Models/Session.h"
#include "ActiveSettings.h"

class SessionStore {

    public:
        SessionStore();

        void load();

        QList<Session> listSessions() const;
        Session getSession(const QString &name) const;





        QStringList listSessionNames() const;

        bool saveSession(const Session &session);
        void deleteSession(const QString &name);

        QStringList listAutoStartSessions() const;
        bool saveAutoStartSessions(const QStringList &names);

    private:
        mutable QSettings settings;

        static Session fromActive(const ActiveSettings &active, const QString &name);
        static void toActive(const Session &session, ActiveSettings &active);

        QMap<QString, Session> sessions;
};

#endif // SESSIONSTORE_H
