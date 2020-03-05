#include "text.h"

Text::Text(QGraphicsItem* parent) : QGraphicsSimpleTextItem(parent)
{
  this->setPos(parent->boundingRect().center());
}

QRectF Text::boundingRect() const
{
    QRectF br = QGraphicsSimpleTextItem::boundingRect();
    return br.translated(-br.width()/2, -br.height()/2);
}

void Text::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    painter->translate(-boundingRect().width()/2, -boundingRect().height()/2);
    QGraphicsSimpleTextItem::paint(painter, option, widget);
}

uchar Text::toUChar()
{
    return this->text().toStdString()[0];
}
