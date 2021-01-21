#include "text.h"
#include <QDebug>

Text::Text(QGraphicsItem* parent) : QGraphicsSimpleTextItem(parent)
{
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

int Text::type() const
{
    return Type;
}
