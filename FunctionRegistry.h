#ifndef FUNCTIONREGISTRY_H
#define FUNCTIONREGISTRY_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QFlags>

enum class UsageContext : quint8 {
    None     = 0x00,
    Keyboard = 0x01,
    Menu     = 0x02,
    Toolbar  = 0x04,
    Script   = 0x08,
    Touch    = 0x10,
    Any      = Keyboard | Menu | Toolbar | Script | Touch
};

Q_DECLARE_FLAGS(UsageContexts, UsageContext)
Q_DECLARE_OPERATORS_FOR_FLAGS(UsageContexts)

struct FunctionInfo {
        QString name;
        UsageContexts contexts;
        QString description;
};

class FunctionRegistry
{
    public:
        static const QList<FunctionInfo> &all();
        static QStringList namesFor(UsageContext context);
        static QList<FunctionInfo> functionsFor(UsageContext context);
};

#endif // FUNCTIONREGISTRY_H
