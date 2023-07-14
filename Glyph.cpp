#include "Glyph.h"
#include <QDebug>

Glyph::Glyph(int x, int y, qreal xscale, qreal yscale, CodePage &cp) : xscale(xscale), yscale(yscale), cp(cp)
{
    pos_x = x;
    pos_y = y;
}

QRectF Glyph::boundingRect() const
{
    return QGraphicsSimpleTextItem::boundingRect();
}

void Glyph::setText(uchar ebcdic)
{
    if (ebcdic == 0x00 || !isDisplay())
    {
        QGraphicsSimpleTextItem::setText(" ");
    }
    else
    {
        if (!graphic)
        {
            QGraphicsSimpleTextItem::setText(cp.getUnicodeChar(ebcdic));
        }
        else
        {
            QGraphicsSimpleTextItem::setText(cp.getUnicodeGraphicChar(ebcdic));
        }
    }

    this->ebcdic = ebcdic;
}

void Glyph::refreshCodePage()
{
    setText(ebcdic);
}

void Glyph::setTextFromKB(uchar ascii)
{
    setText(cp.getEBCDIC(ascii));
}

void Glyph::setCharAttrs(bool c, Glyph::CharAttr ca)
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
        case TRANSPARANCY:
            charAttrTransparency = c;
    }
}

bool Glyph::hasCharAttrs(Glyph::CharAttr ca)
{
    switch(ca)
    {
        case EXTENDED:
            return charAttrExtended;
        case COLOUR:
            return charAttrColour;
        case CHARSET:
            return charAttrCharSet;
        case TRANSPARANCY:
            return charAttrTransparency;
    }

    //TODO: Should never happen, so throw an exception
    return false;
}

void Glyph::resetCharAttrs()
{
    charAttrExtended     = false;
    charAttrColour       = false;
    charAttrCharSet      = false;
    charAttrTransparency = false;
}

int Glyph::type() const
{
    return Type;
}
