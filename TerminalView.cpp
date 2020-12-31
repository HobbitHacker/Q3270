/**
  Terminal

  This class contains all the characteristics of the terminal. Anything that affects the look and feel of the terminal
  should be defined here.
  */

#include "TerminalView.h"

TerminalView::TerminalView()
{
    resizeFont = false;
}


void TerminalView::resizeEvent(QResizeEvent *event)
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

void TerminalView::setScaleFont(bool scale)
{
    resizeFont = scale;
}

void TerminalView::setScenes(DisplayScreen *primary, DisplayScreen *alternate)
{
    this->primary = primary;
    this->alternate = alternate;

}

DisplayScreen *TerminalView::setScreen(bool alt)
{
    if (alt)
    {
        this->setScene(alternate->getScene());
        current = alternate;
    }
    else
    {
        this->setScene(primary->getScene());
        current = primary;
    }
    return current;
}
