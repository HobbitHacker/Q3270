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

#include <QColorDialog>

#include "ColourSwatchWidget.h"
#include "ui_ColourSwatchWidget.h"

ColourSwatchWidget::ColourSwatchWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ColourSwatchWidget)
{
    ui->setupUi(this);

    // Build list of buttons for simpler manipulation later
    colourButtons[Q3270::UnprotectedNormal]     = ui->baseUnprotected;
    colourButtons[Q3270::ProtectedNormal]       = ui->baseProtected;
    colourButtons[Q3270::UnprotectedIntensified]= ui->baseUnprotectedIntensify;
    colourButtons[Q3270::ProtectedIntensified]  = ui->baseProtectedIntensify;

    colourButtons[Q3270::Black]   = ui->colourBlack;
    colourButtons[Q3270::Blue]    = ui->colourBlue;
    colourButtons[Q3270::Red]     = ui->colourRed;
    colourButtons[Q3270::Magenta] = ui->colourPink;
    colourButtons[Q3270::Green]   = ui->colourGreen;
    colourButtons[Q3270::Cyan]    = ui->colourTurq;
    colourButtons[Q3270::Yellow]  = ui->colourYellow;
    colourButtons[Q3270::Neutral] = ui->colourWhite;

    for (QToolButton *btn : colourButtons)
    {
        connect(btn, &QToolButton::clicked, this, &ColourSwatchWidget::setColour);
    }
}

void ColourSwatchWidget::setReadOnly(bool ro)
{
    // Make all QToolButtons inside this widget "read-only"
    const QList<QToolButton *>buttons = findChildren<QToolButton *>();

    for (QToolButton *btn : buttons)
    {
        btn->setAttribute(Qt::WA_TransparentForMouseEvents, ro ? true : false);
        btn->setFocusPolicy(ro ? Qt::NoFocus : Qt::StrongFocus);
    }
}


void ColourSwatchWidget::setTheme(const Colours &theme)
{
    this->theme = theme;
    for (auto it = colourButtons.constBegin(); it != colourButtons.constEnd(); ++it)
    {
        it.value()->setStyleSheet(QStringLiteral("background-color: %1;").arg(theme.colour(it.key()).name()));
    }
}

Colours ColourSwatchWidget::currentTheme() const
{
    return theme;
}

void ColourSwatchWidget::setColour()
{
    QToolButton *buttonSender = qobject_cast<QToolButton*>(sender());
    buttonSender->clearFocus();

    for (auto it = colourButtons.constBegin(); it != colourButtons.constEnd(); ++it)
    {
        if (it.value() == buttonSender)
        {
            Q3270::Colour role = it.key();

            QColor newColour = colourDialog(theme.colour(role), buttonSender);
            if (newColour.isValid())
            {
                theme.setColour(role, newColour);
                emit colourChanged(role, theme.colour(role));
            }
            break;
        }
    }
}

QColor ColourSwatchWidget::colourDialog(QColor c, QToolButton *b)
{
    const QColor dialogColour = QColorDialog::getColor(c, this, tr("Select Colour"));

    if (dialogColour.isValid() && c != dialogColour)
    {
        b->setStyleSheet(QStringLiteral("background-color: %1;").arg(dialogColour.name()));
        return dialogColour;
    }

    return QColor();
}
