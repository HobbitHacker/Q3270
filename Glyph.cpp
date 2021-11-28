#include "Glyph.h"
#include <QDebug>

Glyph::Glyph(int x, int y, QGraphicsItem* parent) : QGraphicsSimpleTextItem(parent)
{
    pos_x = x;
    pos_y = y;
}

QRectF Glyph::boundingRect() const
{
    return QGraphicsSimpleTextItem::boundingRect();
}

void Glyph::setText(const QString text, unsigned char ebcdic, bool graphic)
{
    if (ebcdic == 0x00)
    {
        QGraphicsSimpleTextItem::setText(" ");
    }
    else
    {
        QGraphicsSimpleTextItem::setText(text);
    }

    this->graphic = graphic;
    this->ebcdic = ebcdic;
}

int Glyph::type() const
{
    return Type;
}

/*void Glyph::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
    painter->translate(-boundingRect().width() / 2, -boundingRect().height() / 2);
    QGraphicsSimpleTextItem::paint(painter, option, widget);
}*/
