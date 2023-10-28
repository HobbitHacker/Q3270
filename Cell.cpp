/*

Copyright Ⓒ 2023 Andy Styles
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

#include "Cell.h"
#include <QDebug>
#include <QPen>
#include <QFont>
#include <QFontMetricsF>
#include <QGraphicsItem>
#include <QGraphicsScene>

Cell::Cell(qreal x_pos, qreal y_pos, qreal x, qreal y, CodePage &cp, ColourTheme::Colours &palette, QGraphicsItem *parent, QGraphicsScene *scene) : xsize(x), ysize(y), palette(palette), cp(cp)
{
    QPen p;
    p.setStyle(Qt::NoPen);
    p.setCosmetic(true);

    this->setParentItem(parent);

    this->setRect(0, 0, x, y);
    this->setPen(p);
    this->setZValue(0);
    this->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
    this->setPos(x_pos, y_pos);

    glyph.setBrush(QColor(Qt::white));
    glyph.setParentItem(this);
    glyph.setPos(x_pos, y_pos);
    glyph.setZValue(2);

    underscore.setLine(0, 0, x, 0);
    underscore.setPos(x_pos, y_pos + (y *.98));
    underscore.setVisible(false);
    underscore.setZValue(3);

    scene->addItem(&glyph);
    scene->addItem(&underscore);
}

QRectF Cell::boundingRect() const
{
    return QRectF(0, 0, xsize, ysize);
}

void Cell::setUScore(bool onoff)
{
    uscore = onoff;
    underscore.setPen(QPen(QColor(palette[colNum]), 0));

    if (!fieldStart)
    {
        underscore.setVisible(onoff);
    }
    else
    {
        underscore.setVisible(false);
    }

}

void Cell::setChar(uchar ebcdic)
{
    if (ebcdic == 0x00 || !isDisplay())
    {
        glyph.setText(" ");
    }
    else
    {
        if (!graphic)
        {
            glyph.setText(cp.getUnicodeChar(ebcdic));
        }
        else
        {
            glyph.setText(cp.getUnicodeGraphicChar(ebcdic));
        }
    }

    this->ebcdic = ebcdic;
/*
    int thisc = cp.getUnicodeChar(ebcdic).toLatin1().data()[0];
    if (std::isprint(thisc))
    {
        printf("%c", thisc);
    }
    else
    {
        printf("0x%02X", thisc);
    }
*/
}

void Cell::refreshCodePage()
{
    setChar(ebcdic);
}

void Cell::setCharFromKB(uchar ascii)
{
    setChar(cp.getEBCDIC(ascii));
}

void Cell::setColour(ColourTheme::Colour c)
{
    colNum = c;

    if (fieldStart)
    {
        return;
    }

    if (reverse)
    {
        glyph.setBrush(palette[ColourTheme::Colour::BLACK]);
        this->setBrush(palette[colNum]);
    }
    else
    {
        glyph.setBrush(palette[colNum]);
        underscore.setPen(QPen(palette[colNum], 0));
        this->setBrush(palette[ColourTheme::Colour::BLACK]);
    }
}

void Cell::setFieldStart(bool fs)
{
    fieldStart = fs;

    if (fieldStart)
    {
        setUScore(false);
        setReverse(false);
        setBlink(false);
    }
}

void Cell::setNumeric(bool n)
{
    num = n;
}

void Cell::setGraphic(bool ge)
{
    graphic = ge;
}

void Cell::setMDT(bool m)
{
    mdt = m;
}

void Cell::setProtected(bool p)
{
    prot = p;
}

void Cell::setDisplay(bool d)
{
    display = d;

    if (display)
    {
        glyph.setVisible(true);
        underscore.setVisible(uscore);
    }
    else
    {
        glyph.setVisible(false);
        underscore.setVisible(false);
    }

    glyph.setVisible(true);
}

void Cell::setPenSelect(bool p)
{
    pen = p;
}

void Cell::setIntensify(bool i)
{
    intensify = i;
}

void Cell::setExtended(bool e)
{
    extended = e;
}

void Cell::setReverse(bool r)
{
    reverse = r;

    if (fieldStart)
    {
        return;
    }

    if (reverse)
    {
        this->setBrush(palette[colNum]);
        glyph.setBrush(palette[ColourTheme::Colour::BLACK]);
    }
    else
    {
        glyph.setBrush(palette[colNum]);
        this->setBrush(palette[ColourTheme::Colour::BLACK]);
    }
}

void Cell::setBlink(bool b)
{
    blink = b;
}

void Cell::setCharAttrs(bool c, Cell::CharAttr ca)
{
    switch(ca)
    {
        case EXTENDED:
            charAttrExtended = c;
            break;
        case COLOUR:
            charAttrColour = c;
            break;
        case CHARSET:
            charAttrCharSet = c;
            break;
        case TRANSPARENCY:
            charAttrTransparency = c;
    }
}

bool Cell::hasCharAttrs(Cell::CharAttr ca)
{
    switch(ca)
    {
        case EXTENDED:
            return charAttrExtended;
        case COLOUR:
            return charAttrColour;
        case CHARSET:
            return charAttrCharSet;
        case TRANSPARENCY:
            return charAttrTransparency;
    }

    //TODO: Should never happen, so throw an exception
    return false;
}

void Cell::resetCharAttrs()
{
    charAttrExtended     = false;
    charAttrColour       = false;
    charAttrCharSet      = false;
    charAttrTransparency = false;
}

void Cell::copyAttrs(Cell *fromCell)
{
    prot = fromCell->isProtected();
    mdt = fromCell->isMdtOn();
    num = fromCell->isNumeric();
    pen = fromCell->isPenSelect();
    blink = fromCell->isBlink();

    setDisplay(fromCell->isDisplay());
    setColour(fromCell->getColour());
    setUScore(fromCell->isUScore());
    setReverse(fromCell->isReverse());

    resetCharAttrs();
}

void Cell::setAttrs(bool prot, bool mdt, bool num, bool pensel, bool blink, bool disp, bool under, bool rev, ColourTheme::Colour col)
{
    setProtected(prot);
    setMDT(mdt);
    setNumeric(num);
    setPenSelect(pensel);
    setBlink(blink);
    setDisplay(disp);
    setUScore(under);
    setReverse(rev);
    setColour(col);
}

int Cell::type() const
{
    return Type;
}

void Cell::blinkChar(bool blink)
{
    if (blink)
    {
        glyph.setBrush(palette[colNum]);
    }
    else
    {
        glyph.setBrush(palette[ColourTheme::Colour::BLACK]);
    }
}

void Cell::setFont(QFont f)
{
    QFontMetricsF fm = QFontMetrics(f);

    qreal xs;
    qreal ys;

    f.setStyleStrategy( QFont::NoFontMerging);
    f.setStyleStrategy(QFont::NoSubpixelAntialias);

    xs = fm.horizontalAdvance("┼", 1);
    ys = fm.height();

//    xs = fm.boundingRect("┼").width();
//g    ys = fm.boundingRect("┼").height();

    QTransform fontScale;

    fontScale.scale(xsize / xs, ysize / ys);

    glyph.setTransform(fontScale);

    glyph.setFont(f);
}
