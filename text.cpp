#include "text.h"
#include <QDebug>

Text::Text(int x, int y, QGraphicsItem* parent) : QGraphicsSimpleTextItem(parent)
{
    pos_x = x;
    pos_y = y;
}

QRectF Text::boundingRect() const
{
    return QGraphicsSimpleTextItem::boundingRect();
}

void Text::setText(const QString text, unsigned char ebcdic, bool graphic)
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

int Text::type() const
{
    return Type;
}
