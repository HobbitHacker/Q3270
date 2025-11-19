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

/**
 * @brief   Format a host address string.
 * @param   hostName    The hostname or IP address.
 * @param   hostPort    The port number (0 if not specified).
 * @param   hostLU      The LU name (empty if not specified).
 * @return  A formatted address string.
 *
 * @details This function constructs a host address string in the format:
 *          [LU@]hostname[:port]
 */
QString HostAddressUtils::format(const QString &hostName, int hostPort, const QString &hostLU) {
    QString address;

    if (!hostLU.isEmpty())
        address.append(hostLU).append('@');

    if (!hostName.isEmpty())
        address.append(hostName);
    else
        return "";

    if (hostPort != 0)
        address.append(':').append(QString::number(hostPort));

    return address;
}

/**
 * @brief   Parse a host address string.
 * @param   address     The address string to parse.
 * @param   hostName    Output parameter for the hostname or IP address.
 * @param   hostPort    Output parameter for the port number.
 * @param   hostLU      Output parameter for the LU name.
 *
 * @details This function extracts the hostname, port, and LU name
 *          from an address string in the format: [LU@]hostname[:port]
 */
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
