#ifndef CLICKABLESVGITEM_H
#define CLICKABLESVGITEM_H

#include <QGraphicsSvgItem>

class ClickableSvgItem : public QGraphicsSvgItem
{
    public:
        using QGraphicsSvgItem::QGraphicsSvgItem;

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif // CLICKABLESVGITEM_H
