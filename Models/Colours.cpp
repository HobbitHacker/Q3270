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
