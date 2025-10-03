/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef HOSTADDRESSUTILS_H
#define HOSTADDRESSUTILS_H

#include <QString>

namespace HostAddressUtils
{
    QString format(const QString &hostName, int hostPort, const QString &hostLU);
    void parse(const QString &address, QString &hostName, int &hostPort, QString &hostLU);
}

#endif // HOSTADDRESSUTILS_H
