#ifndef KEYBOARDMAP_H
#define KEYBOARDMAP_H

#include <QString>
#include <QStringList>
#include <QKeySequence>

struct Mapping
{
    QString functionName;
    QStringList keys;

};

struct KeyboardMap
{
    QString name;
    QList<Mapping> mappings;

    static KeyboardMap getFactoryMap();
    void set(const QString &function, const QStringList &keys);
    QStringList getFunctions() const;

    void forEach(std::function<void(const QString&, const QStringList&)> fn) const;
    void setKeyMapping(const QString &functionName, const QKeySequence &sequence);

    void dumpMaps(const QString &tag) const;

};

#endif // KEYBOARDMAP_H
