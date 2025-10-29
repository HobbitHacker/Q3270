/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "DisplayScreen.h"

/**
 * @brief   DisplayScreen::mousePressEvent - process a mouse event
 * @param   mEvent - the event
 *
 * @details Called when a mouse event happens in DisplayScreen. This routine handles a left-click, and
 *          stores the coordinates of the click. The Rubberband is hidden (it will be shown if the mouse
 *          is moved).
 */
void DisplayScreen::mousePressEvent(QGraphicsSceneMouseEvent *mEvent)
{
    if (mEvent->button() != Qt::LeftButton)
    {
        mEvent->ignore();
        return;
    }

    int x = mEvent->pos().x() / gridSize_X;
    int y = mEvent->pos().y() / gridSize_Y;

//    qDebug() << "Mouse press at" << mEvent->pos() << "- char pos" << x << "," << y << "scaled pos" << x * gridSize_X << "," << y * gridSize_Y;

    mouseStart = mapFromItem(this, QPoint(x * gridSize_X, y * gridSize_Y));

    myRb->setData(0, x);
    myRb->setData(1, y);
    myRb->setData(2, x);
    myRb->setData(3, y);

    myRb->hide();
}

/**
 * @brief   DisplayScreen::mouseMoveEvent - process a mouse move event
 * @param   mEvent - the event
 *
 * @details Called when the mouse is moved after a click. This routine calculates the Cells around which
 *          the rubberband is to be drawn and then makes it visible.
 */
void DisplayScreen::mouseMoveEvent(QGraphicsSceneMouseEvent *mEvent)
{   
    //FIXME: Some of this could probably be simplified so we're not working out
    //       the min/max values in mouseReleaseEvent

    // Calculate character position of mouse
    int thisX = mEvent->pos().x() / gridSize_X;
    int thisY = mEvent->pos().y() / gridSize_Y;

    // Normalise the position to within the bounds of the character display
    thisX = std::min(thisX, screen_x - 1);
    thisY = std::min(thisY, screen_y - 1);

    thisX = std::max(thisX, 0);
    thisY = std::max(thisY, 0);

    // Retrieve the start point of the selection
    int mpX = myRb->data(0).toInt();
    int mpY = myRb->data(1).toInt();

    /* Normalise the new mouse position
       If the user moves the mouse up and/or left, this is the new start point,
       and the old start point becomes the bottom right
    */
    int topLeftX = std::min(mpX, thisX);
    int topLeftY = std::min(mpY, thisY);

    int botRightX = std::max(thisX, mpX);
    int botRightY = std::max(thisY, mpY);

    myRb->setData(2, thisX);
    myRb->setData(3, thisY);

    // Add one to sizing to ensure rectangle moves when mouse moves to next character
    int w = botRightX - topLeftX + 1;
    int h = botRightY - topLeftY + 1;

//    qDebug() << "Move" << mEvent->pos() << "this " << thisX << "," << thisY << " mpX,mpY" << mpX << "," << mpY << "    topLeftX,topLeftY" << topLeftX << "," << topLeftY << "    botRightX,botRightY" << botRightX << "," << botRightY << "w,h" << w << "x" << h;

    myRb->setRect(topLeftX * gridSize_X, topLeftY * gridSize_Y, w * gridSize_X, h * gridSize_Y);

    myRb->show();
}

/**
 * @brief   DisplayScreen::mouseReleaseEvent - process a mouse release event
 * @param   mEvent - the event
 *
 * @details Called when the left mouse button is released. If the mouse button was released without
 *          moving the mouse, the rubberband will be invisible, and this is interpreted as the user
 *          wishing to move the cursor by clicking somewhere in the display.
 */
void DisplayScreen::mouseReleaseEvent(QGraphicsSceneMouseEvent *mEvent)
{
//    qDebug() << "Mouse release at " << mEvent->pos();

    // Single click, move cursor
    if (!myRb->isVisible())
    {
//        qDebug() << "Single click";
        setCursor(myRb->data(0).toInt(), myRb->data(1).toInt());
        return;
    }

  /*  int top = std::min(myRb->data(1).toInt(), myRb->data(3).toInt());
    int bottom = std::max(myRb->data(1).toInt(), myRb->data(3).toInt());

    int left = std::min(myRb->data(0).toInt(), myRb->data(2).toInt());
    int right = std::max(myRb->data(0).toInt(), myRb->data(2).toInt());
*/
//    qDebug() << "Selected" << left << "," << top << "x" << right << "," << bottom;
}

/**
 * @brief   DisplayScreen::copyText - copy the text within the rubberband to the clipboard
 *
 * @details Called when the user invokes the Copy function (default Ctrl-C) to copy the text
 *          contained within the rubberband region to the clipboard. Each new line within the
 *          rubberband generates a newline on the clipboard.
 */
void DisplayScreen::copyText()
{
    // If the rubberband isn't showing, do nothing
    if (!myRb->isVisible()) {
        return;
    }

    // Build up a string with the selected characters
    QString cbText = "";

    int top = std::min(myRb->data(1).toInt(), myRb->data(3).toInt());
    int bottom = std::max(myRb->data(1).toInt(), myRb->data(3).toInt());

    int left = std::min(myRb->data(0).toInt(), myRb->data(2).toInt());
    int right = std::max(myRb->data(0).toInt(), myRb->data(2).toInt());

    qDebug() << "Selection " << top << "," << left << " x " << bottom << "," << right;

    for(int y = top; y <= bottom; y++)
    {
        // Append a newline if there's more than one row selected
        if (y > top) {
            cbText = cbText + "\n";
        }

        for(int x = left; x <= right; x++)
        {
            int thisPos = screen_x * y + x;
            cbText = cbText + cp.getUnicodeChar(cells[thisPos].getEBCDIC());
        }
    }

    qDebug() << "Clipboard text: " << cbText;

    QClipboard *clipboard = QGuiApplication::clipboard();

    clipboard->setText(cbText);

    myRb->hide();
}
