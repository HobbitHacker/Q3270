#ifndef STRUCTUREDFIELD_H
#define STRUCTUREDFIELD_H

#include <QByteArray>

class StructuredField
{
    public:
        explicit StructuredField(quint8 type);
        StructuredField &addByte(quint8 b);
        StructuredField &addHalfWord(quint16 w); // big-endian 16-bit, as the 3270 stream expects
        StructuredField &addWord(quint32 f);     // big-endian 32-bit, as the 3270 stream expects
        StructuredField &addBytes(std::initializer_list<quint8> bytes);

        // Wraps what's been added so far with its own length prefix.
        QByteArray toSF() const;

    private:
        QByteArray data;
};


#endif // STRUCTUREDFIELD_H
