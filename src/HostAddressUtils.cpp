/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "HostAddressUtils.h"

QString HostAddressUtils::format(const QString &hostName, int hostPort, const QString &hostLU) {
    QString address;
    if (!hostLU.isEmpty())
        address.append(hostLU).append('@');
    if (!hostName.isEmpty())
        address.append(hostName);
    if (hostPort != 0)
        address.append(':').append(QString::number(hostPort));
    return address;
}

void HostAddressUtils::parse(const QString &address, QString &hostName, int &hostPort, QString &hostLU) {
    if (address.contains('@')) {
        hostLU   = address.section('@', 0, 0);
        hostName = address.section('@', 1, 1).section(':', 0, 0);
        hostPort = address.section(':', 1, 1).toInt();
    } else {
        hostLU   = "";
        hostName = address.section(':', 0, 0);
        hostPort = address.section(':', 1, 1).toInt();
    }
}
