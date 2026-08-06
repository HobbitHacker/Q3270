/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include <QByteArray>

#include "StructuredField.h"
/**
 *  StructuredField - A utility class to construct 3270 Structured Fields for the data stream.
 **/

/**
 * @brief StructuredField - Construct a StructuredField with a specific type.
 * @param type - the type of the structured field
 */
StructuredField::StructuredField(quint8 type)
{
    addByte(type);
}

/**
 * @brief   StructuredField::addByte - Add a single byte to the structured field
 * @param   b - the byte to add
 * @return  A reference to the StructuredField object, allowing for method chaining
 */
StructuredField &StructuredField::addByte(quint8 b)
{
    data.append(static_cast<char>(b));
    return *this;
}

/**
 * @brief  StructuredField::addHalfWord - Add a 16-bit word to the structured field in big-endian order
 * @param  w - the 16-bit word to add
 * @return A reference to the StructuredField object, allowing for method chaining
 */
StructuredField &StructuredField::addHalfWord(quint16 w)   // big-endian 16-bit, as the 3270 stream expects
{
    data.append(static_cast<char>((w >> 8) & 0xFF));
    data.append(static_cast<char>(w & 0xFF));
    return *this;
}

/**
 * @brief StructuredField::addWord - Add a 32-bit word to the structured field in big-endian order
 * @param f - the 32-bit word to add
 * @return A reference to the StructuredField object, allowing for method chaining
 */
StructuredField &StructuredField::addWord(quint32 f)
{
    data.append(static_cast<char>((f >> 24) & 0xFF));
    data.append(static_cast<char>((f >> 16) & 0xFF));
    data.append(static_cast<char>((f >> 8) & 0xFF));
    data.append(static_cast<char>(f & 0xFF));
    return *this;
}

/**
 * @brief  StructuredField::addBytes - Add multiple bytes to the structured field from
 *         an initializer list
 *
 * @param  bytes - an initializer list of bytes to add
 * @return A reference to the StructuredField object, allowing for method chaining
 */
StructuredField &StructuredField::addBytes(std::initializer_list<quint8> bytes)
{
    for (quint8 b : bytes)
        data.append(static_cast<char>(b));

    return *this;
}

/**
 * @brief  StructuredField::toSF - Convert the structured field to a QByteArray with a length prefix
 * @return A QByteArray containing the structured field data, prefixed with its length
 */
QByteArray StructuredField::toSF() const
{
    quint16 len = data.size() + 2;
    QByteArray sf;

    sf.append(static_cast<uchar>((len >> 8) & 0xFF));
    sf.append(static_cast<uchar>(len & 0xFF));

    sf.append(data);
    return sf;
}
