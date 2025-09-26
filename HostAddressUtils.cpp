/*

Copyright Ⓒ 2025 Andy Styles
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
