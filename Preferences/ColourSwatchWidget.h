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

#ifndef COLOURSWATCHWIDGET_H
#define COLOURSWATCHWIDGET_H

#include <QToolButton>

#include "Models/Colours.h"

namespace Ui
{
    class ColourSwatchWidget;
}

class ColourSwatchWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit ColourSwatchWidget(QWidget *parent = nullptr);

        void setTheme(const Colours &theme);
        void setReadOnly(bool ro);
        Colours currentTheme() const;

    signals:
        void colourChanged(Q3270::Colour role, const QColor &newColour);

    private slots:
        void setColour(); // slot for swatch clicks

    private:
        QColor colourDialog(QColor c, QToolButton *b);

        Ui::ColourSwatchWidget *ui;

        QMap<Q3270::Colour, QToolButton*> colourButtons;
        Colours theme;
};

#endif // COLOURSWATCHWIDGET_H
