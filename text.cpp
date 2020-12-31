#include "text.h"
#include <QDebug>

Text::Text(QGraphicsItem* parent) : QGraphicsSimpleTextItem(parent)
{
//  this->setPos(parent->boundingRect().center());
    this->setPos(0,0);
}

QRectF Text::boundingRect() const
{
    QRectF br = QGraphicsSimpleTextItem::boundingRect();
//    printf("Text BoundingRect: %f x %f (translated %f x %f)\n", br.width(), br.height(), br1.width(), br1.height());
//    fflush(stdout);
//    return br.translated(-br.width() / 2, -br.height() / 2);

      return br;
//    return QGraphicsSimpleTextItem::boundingRect();
}

void Text::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    //painter->translate(-boundingRect().width()/2, -boundingRect().height()/2);
    QGraphicsSimpleTextItem::paint(painter, option, widget);
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
        QGraphicsSimpleTextItem::setText(0x00);
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
