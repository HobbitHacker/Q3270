#ifndef LOCKINDICATOR_H
#define LOCKINDICATOR_H

#include <QGraphicsItemGroup>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsSvgItem>
#include <QString>

class LockIndicator : public QGraphicsItemGroup
{
    public:
        enum Mode { None, Clock, System };

        explicit LockIndicator(QGraphicsItem* parent = nullptr);
        void setMode(Mode m);

        /**
         * @brief    mode - return the current lock indicator
         * @return   LockIndicator::Mode representing the currently displayed indicator
         */
        Mode mode() const;

        void paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*);

    private:

        QGraphicsSimpleTextItem* xText;
        QGraphicsSvgItem*  clockSvg;

        QGraphicsSimpleTextItem* systemText;
        Mode currentMode;
};

#endif // LOCKINDICATOR_H
