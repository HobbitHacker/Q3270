#ifndef HOSTADDRESSUTILS_H
#define HOSTADDRESSUTILS_H

#include <QString>

namespace HostAddressUtils
{
    QString format(const QString &hostName, int hostPort, const QString &hostLU);
    void parse(const QString &address, QString &hostName, int &hostPort, QString &hostLU);
}

#endif // HOSTADDRESSUTILS_H
