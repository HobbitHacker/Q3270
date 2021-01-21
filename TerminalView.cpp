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




void TerminalView::mousePressEvent(QMouseEvent *event)
{
    origin = event->pos();
//    QGraphicsItem *item = current->itemAt(mapToScene(event->pos()), QTransform());

//    if (item->type() == Text::Type)
    //{
        //printf(">> %2X <<", ((Text *)item)->getEBCDIC());
//        item->parentItem()->setSelected(true);
    //}
//    QRectF a = item->sceneBoundingRect();
//    QPoint w = QPoint(current->gridWidth() + origin.x(), current->gridHeight() + origin.y());
    rubberBand->setGeometry(QRect(origin, QSize()));
    printf("Click at %d, %d - %f x %f\n", origin.x(), origin.y(), current->gridWidth(), current->gridWidth());
//    printf("Item (To Scene) at %f, %f\n", item->pos().x(), item->pos().y());
//    printf("A (To Scene) at %f, %f - %f x %f\n", a.x(), a.y(), a.width(), a.height());
    printf("RubberBand at %d,%d\n", rubberBand->x(), rubberBand->y());
    fflush(stdout);
    rubberBand->show();
}

void TerminalView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint thisPoint = event->pos();
    QPoint w = QPoint(current->gridWidth() + thisPoint.x(), current->gridHeight() + thisPoint.y());
    rubberBand->setGeometry(QRect(origin, thisPoint).normalized());
/*    rubberBand->setRect()
    printf("New size %dx%d\n", rubberBand->width(), rubberBand->height());
    fflush(stdout);*/
}

void TerminalView::mouseReleaseEvent(QMouseEvent *event)
{
    rubberBand->hide();
    QRect rect = rubberBand->geometry();
//    QRectF rect_scene = mapToScene(rect).bou;
//    selected = items(rect_scene)
    printf("RubberBand: %dx%d at %d,%d\n", rubberBand->width(), rubberBand->height(), rubberBand->x(), rubberBand->y());
    QList<QGraphicsItem *>cells = current->items(mapToScene(rect), Qt::IntersectsItemShape, Qt::AscendingOrder);
    printf("Items : %d\n", cells.size());
    fflush(stdout);
    for(int i = 0; i < cells.size(); i++)
    {
        qDebug() << cells.at(i)->type();
        if (cells.at(i)->type() == Text::Type)
        {
            Text *a = dynamic_cast<Text *>(cells.at(i));
            a->setSelected(true);

            qDebug() << a->text();
            printf("Item %i = %X, at %f,%f - %2X\n", i, a, a->pos().x(), a->pos().y(), a->getEBCDIC());
            fflush(stdout);
        }
        else
        {
//            cells.at(i)->setSelected(true);
        }
    }
/*
    cells = items();
    for(int i = 0; i < cells.size(); i++)
    {
        printf("Item %d at %f,%f\n", i, cells.at(i)->pos().x(), cells.at(i)->pos().y());
    }*/
    // determine selection, for example using QRect::intersects()
    // and QRect::contains().
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
