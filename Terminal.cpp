/**
  Terminal

  This class contains all the characteristics of the terminal. Anything that affects the look and feel of the terminal
  should be defined here.
  */

#include "Terminal.h"

Terminal::Terminal()
{
    setType("IBM-3279-2-E");
    setBlink(true);
    setBlinkSpeed(1000);
    setScaleFont(true);
}

void Terminal::setType(QString type)
{
    for (int i = 0; i < 5; i++)
    {
        if (type == terms[i].term)
        {
            termType = i;
            return;
        }
    }

    termType = 0;
}

void Terminal::setType(int type)
{
    termType = type;
}

int Terminal::terminalWidth()
{
    return terms[termType].x;
}

int Terminal::terminalHeight()
{
    return terms[termType].y;
}

void Terminal::setSize(int x, int y)
{
    if (termType != 4)
    {
        return;
    }

    terms[4].x = x;
    terms[4].y = y;
}

int Terminal::getType()
{
    return termType;
}

char * Terminal::name()
{
    return terms[termType].term.toLatin1().data();
}

void Terminal::setBlink(bool b)
{
    blink = b;
}

void Terminal::setBlinkSpeed(int speed)
{
    if (blinkSpeed > 1000 || blinkSpeed < 0)
    {
        return;
    }

    blinkSpeed = speed;
    emit cursorBlinkChange();
}

bool Terminal::getBlink()
{
    return blink;
}

int Terminal::getBlinkSpeed()
{
    return blinkSpeed;
}

void Terminal::resizeEvent(QResizeEvent *event)
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

void Terminal::setScaleFont(bool scale)
{
    if(resizeFont != scale)
    {
        resizeFont = scale;
    }
}
