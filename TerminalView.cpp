/**
  Terminal

  This class contains all the characteristics of the terminal. Anything that affects the look and feel of the terminal
  should be defined here.
  */

#include "TerminalView.h"

TerminalView::TerminalView()
{
    stretch = false;
    blinker = new QTimer(this);
    cursorBlinker = new QTimer(this);
    blinkSpeed = 1;
    blink = true;
    connected = false;
}


void TerminalView::resizeEvent(QResizeEvent *event)
{

    if (stretch)
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
    scaleFont = scale;
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

    connect(blinker, &QTimer::timeout, current, &DisplayScreen::blink);
    blinker->start(1000);

    connect(cursorBlinker, &QTimer::timeout, current, &DisplayScreen::cursorBlink);
    setBlinkSpeed(blinkSpeed);

    return current;
}

void TerminalView::blinkText()
{
    current->blink();
}

void TerminalView::blinkCursor()
{
    current->cursorBlink();
}

void TerminalView::setBlinkSpeed(int speed)
{
    blinkSpeed = speed;
    cursorBlinker->stop();
    if (blinkSpeed > 0 && blink)
    {
        cursorBlinker->start((5 - blinkSpeed) * 250);
    }
}

int TerminalView::getBlinkSpeed()
{
    return blinkSpeed;
}

void TerminalView::setBlink(bool blink)
{
    this->blink = blink;
    if (!blink)
    {
        cursorBlinker->stop();
        if (connected)
        {
            current->showCursor();
        }
    }
}

bool TerminalView::getBlink()
{
    return blink;
}

void TerminalView::setConnected()
{
    connected = true;
}

void TerminalView::setDisconnected()
{
    connected = false;
}
