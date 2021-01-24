/**
  Terminal

  This class contains all the characteristics of the terminal. Anything that affects the look and feel of the terminal
  should be defined here.
  */

#include "TerminalView.h"
#include "text.h"

TerminalView::TerminalView()
{
    stretch = false;
    blinker = new QTimer(this);
    cursorBlinker = new QTimer(this);
    blinkSpeed = 1;
    blink = true;
    connected = false;
//    QGraphicsView::setDragMode(QGraphicsView::RubberBandDrag);
    QGraphicsView::setInteractive(true);
    selection = new QList<Text *>();

}


void TerminalView::resizeEvent(QResizeEvent *event)
{

    if (stretch)
    {
        fitInView(this->scene()->itemsBoundingRect(), Qt::IgnoreAspectRatio);
    }
    else
    {
//        fitInView(this->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        fitInView(this->scene()->itemsBoundingRect(), Qt::IgnoreAspectRatio);
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
    setScene(primary);
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
}

DisplayScreen *TerminalView::setScreen(bool alt)
{
    blinker->stop();
    cursorBlinker->stop();
    disconnect(blinker, &QTimer::timeout, current, &DisplayScreen::blink);
    disconnect(cursorBlinker, &QTimer::timeout, current, &DisplayScreen::cursorBlink);

    rubberBand->hide();

    if (alt)
    {
        setScene(alternate);
        current = alternate;
    }
    else
    {
        setScene(primary);
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

void TerminalView::clearSelection()
{
    if (selection->count() > 0)
    {
        for (int i = 0; i < selection->count(); i++)
        {
            selection->at(i)->setSelected(false);
        }
    }
    selection->clear();
}


void TerminalView::mousePressEvent(QMouseEvent *event)
{
    origin = event->pos();

    rubberBand->setGeometry(QRect(origin, QSize()));

    rubberBand->show();

    clearSelection();
}

void TerminalView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint thisPoint = event->pos();
    rubberBand->setGeometry(QRect(origin, thisPoint).normalized());
}

void TerminalView::mouseReleaseEvent(QMouseEvent *event)
{
    rubberBand->hide();
    QRect rect = rubberBand->geometry();
    QList<QGraphicsItem *>cells = current->items(mapToScene(rect), Qt::IntersectsItemShape, Qt::AscendingOrder);
    for(int i = 0; i < cells.size(); i++)
    {
        if (cells.at(i)->type() == Text::Type)
        {
            Text *a = dynamic_cast<Text *>(cells.at(i));
            selection->append(a);
            a->setSelected(true);
        }
    }
    printf("Selected items: %d\n", selection->count());
    fflush(stdout);
}

void TerminalView::copyText()
{
    printf("Selection count: %d\n", selection->count());
    fflush(stdout);
    if (selection->count() == 0)
    {
        return;
    }
    int sy = selection->at(0)->getY();
    QString clip = "";
    for (int i = 0; i < selection->count(); i++)
    {
        if (selection->at(i)->getY() != sy)
        {
            clip = clip + "\n";
            sy = selection->at(i)->getY();
        }
        selection->at(i)->setSelected(false);
        clip = clip + selection->at(i)->text();
    }
    selection->clear();
    QClipboard *cb = QApplication::clipboard();
    cb->setText(clip);
    printf("Text: '%s'\n", clip.toLatin1().data());
    fflush(stdout);
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
