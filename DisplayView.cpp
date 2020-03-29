#include "DisplayView.h"

DisplayView::DisplayView()
{

}

void DisplayView::resizeEvent(QResizeEvent *event)
{

    fitInView(this->scene()->itemsBoundingRect(), Qt::IgnoreAspectRatio);

    QGraphicsView::resizeEvent(event);
}
