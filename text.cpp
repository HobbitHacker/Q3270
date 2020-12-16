#include "text.h"
#include <QDebug>

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
    return this->text().toLatin1()[0];
}

void Text::setText(const QString &text)
{
    if (text.size()>0)
    {
        printf("Text: (%c)\n",text.toLatin1().data()[0]);
        fflush(stdout);
        if (text == '\u0000' || text == '\u001A')
        {
            QGraphicsSimpleTextItem::setText(0x00);
        }
        else
        {
            QGraphicsSimpleTextItem::setText(text);
        }
    }
    else
    {
        QGraphicsSimpleTextItem::setText(0x00);
    }
}
