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

int Text::getX()
{
    return pos_x;
}

int Text::getY()
{
    return pos_y;
}

uchar Text::getEBCDIC()
{
    return ebcdic;
}

bool Text::getGraphic()
{
    return graphic;
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

void Text::copyTextFrom(Text *source)
{
    QGraphicsSimpleTextItem::setText(source->text());
    graphic = source->getGraphic();
    ebcdic = source->getEBCDIC();
}

int Text::type() const
{
    return Type;
}
