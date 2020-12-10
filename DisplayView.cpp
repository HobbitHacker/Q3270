#include "DisplayView.h"

DisplayView::DisplayView()
{
    resizeFont = true;
}

void DisplayView::resizeEvent(QResizeEvent *event)
{

    if (resizeFont)
    {
        fitInView(this->scene()->itemsBoundingRect(), Qt::IgnoreAspectRatio);
    }
    else
    {
        fitInView(this->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
    }

    QGraphicsView::resizeEvent(event);
}

void DisplayView::scaleFont(bool scale)
{
    if(resizeFont != scale)
    {
        resizeFont = scale;
    }
}

