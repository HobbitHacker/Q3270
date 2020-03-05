#ifndef TEXT_H
#define TEXT_H

#include <QRectF>
#include <QObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSimpleTextItem>

class Text : public QObject, public QGraphicsSimpleTextItem
{

    Q_OBJECT

    public:
        Text(QGraphicsItem* parent = 0);
        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
        uchar toUChar();
};

#endif // TEXT_H
