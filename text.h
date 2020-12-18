#ifndef TEXT_H
#define TEXT_H

#include <QRectF>
#include <QObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSimpleTextItem>

#include "3270.h"

class Text : public QObject, public QGraphicsSimpleTextItem
{

    Q_OBJECT

    public:
        Text(QGraphicsItem* parent = 0);
        QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
        uchar getEBCDIC();
        bool getGraphic();
        void setText(const QString text, uchar ebcdic, bool graphic);
        void copyTextFrom(Text *source);

   private:

        QString contents;
        uchar ebcdic;
        bool graphic;
};

#endif // TEXT_H
