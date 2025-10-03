/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "Colours.h"

Colours Colours::getFactoryTheme()
{
    Colours c;

    c.name = "Factory";

    c.map[Q3270::UnprotectedNormal]      = QColor(0,255,0);
    c.map[Q3270::ProtectedNormal]        = QColor(128,128,255);
    c.map[Q3270::UnprotectedIntensified] = QColor(255,0,0);
    c.map[Q3270::ProtectedIntensified]   = QColor(255,255,255);

    c.map[Q3270::Black]   = QColor(0,0,0);
    c.map[Q3270::Blue]    = QColor(128,128,255);
    c.map[Q3270::Red]     = QColor(255,0,0);
    c.map[Q3270::Magenta] = QColor(255,0,255);
    c.map[Q3270::Green]   = QColor(0,255,0);
    c.map[Q3270::Cyan]    = QColor(0,255,255);
    c.map[Q3270::Yellow]  = QColor(255,255,0);
    c.map[Q3270::Neutral] = QColor(255,255,255);

    return c;
}

QColor Colours::colour(Q3270::Colour role) const
{
    return map.value(role);
}

void Colours::setColour(Q3270::Colour role, const QColor &c)
{
    map[role] = c;
}
