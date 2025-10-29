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
 * @brief   DisplayScreen::setCursor - position cursor
 * @param   cursorPos - screen position
 *
 * @details setCursor is used when the cursor is moved either by the user or by the incoming 3270
 *          data stream.
 */
void DisplayScreen::setCursor(int cursorPos)
{
    int cursor_y = (cursorPos / screen_x);
    int cursor_x = cursorPos - (cursor_y * screen_x);

    setCursor(cursor_x, cursor_y);
}

/**
 * @brief   DisplayScreen::setCursor - position cursor
 * @param   x - screen position x
 * @param   y - screen position y
 *
 * @details setCursor is used when the cursor is moved either by the user or by the incoming 3270
 *          data stream.
 */
void DisplayScreen::setCursor(const int x, const int y)
{
    cursor.setVisible(false);

    cursor_pos = x + (y * screen_x);

    if (cursorColour)
    {
        const Cell c = cells[cursor_pos];
        const Q3270::Colour colour = c.isReverse() ? Q3270::Black : c.getColour();

        cursor.setBrush(palette->colour(colour));
    }

    cursor.setPos(gridSize_X * (qreal) x, gridSize_Y * (qreal) y);
//    cursor.setData(0,pos);

    cursor.setVisible(true);

    emit cursorMoved(x + 1, y + 1);

    setRuler();
}

/**
 * @brief   DisplayScreen::moveCursor - move the cursor
 * @param   x        - x position to move the cursor to
 * @param   y        - y position to move the cursor to

 */
void DisplayScreen::moveCursor(int x, int y)
{
    int tmpCursorPos = (cursor_pos + (y * screen_x + x)) % screenPos_max;

    if (tmpCursorPos < 0)
    {
        tmpCursorPos += screenPos_max;
    }

    setCursor(tmpCursorPos);
}

/**
 * @brief   DisplayScreen::showCursor - display cursor
 *
 * @details Called when the cursor blink is switched off to ensure that the cursor doesn't
 *          remain hidden if the blink happened to be at the point the cursor was hidden.
 */
void DisplayScreen::showCursor()
{
    cursor.show();
}

/**
 * @brief   DisplayScreen::newline - move the cursor to the first input field after the current line
 *
 * @details Move the cursor to the first input field found after the start of the next line.
 */
void DisplayScreen::newline()
{
    int cursor_y = (cursor_pos / screen_x) + 1;

    if (cursor_y > screen_y)
    {
        cursor_y = 0;
    }

    cursor_pos = cursor_y * screen_x;

    tab(0);
}

/**
 * @brief   DisplayScreen::tab - tab to the next field
 * @param   offset - offset from the current position
 *
 * @details Move the cursor to the next input field, skipping the attribute byte.
 */
void DisplayScreen::tab(int offset)
{
    // Move cursor to next unprotected field, plus one to skip Field Start byte
    setCursor((findNextUnprotectedField(cursor_pos + offset) + 1) % screenPos_max);
}

/**
 * @brief   DisplayScreen::backtab - back to the previous field start
 *
 * @details Move the cursor to the start of the previous field (which may be this field)
 */
void DisplayScreen::backtab()
{
    setCursor((findPrevUnprotectedField(cursor_pos) + 1) % screenPos_max);
}

/**
 * @brief   DisplayScreen::endline - move the cursor to the end of the current input field
 *
 * @details Move the cursor to the end of the text in the current input field.
 */
void DisplayScreen::endline()
{
    if (isProtected(cursor_pos))
    {
        return;
    }

    int endPos = cursor_pos + screenPos_max;

    int endField;

    int i = cursor_pos;
    int offset = cursor_pos;

    endField = cursor_pos;
    bool letter = false;

    while(i < endPos && !isProtected(offset) && !isFieldStart(offset))
    {
        uchar thisChar = cp.getEBCDIC(cells[offset].getEBCDIC());
        if (letter && (thisChar == 0x00 || thisChar == ' '))
        {
            endField = offset;
            letter = false;
        }

        if (thisChar != 0x00 && thisChar != ' ')
        {
            letter = true;
        }

        offset = ++i % screenPos_max;
    }

    setCursor(endField);
}

/**
 * @brief   DisplayScreen::home - move the cursor to the first field on the screen
 *
 * @details Move the cursor to the first field on the screen; searching starts at the very last position
 *          in case that is a field start, and the first position is not.
 */
void DisplayScreen::home()
{
    // Find first field on screen; this might be position 0, so we need to look starting at the last screen pos
    int nf = (findNextUnprotectedField(screenPos_max - 1) + 1) % screenPos_max;
    int cursor_y = (nf / screen_x);
    int cursor_x = nf - (cursor_y * screen_x);

    // Move cursor right (skip attribute byte)
    setCursor(cursor_x, cursor_y);
}

/**
 * @brief   DisplayScreen::backspace - backspace one character
 *
 * @details Backspace one character, stopping at the field start
 */
void DisplayScreen::backspace()
{
    // If we're at a protected field, do nothing
    if (isProtected(cursor_pos))
        return;

    // Discover whether the previous cursor position is a field start
    int tempCursorPos = cursor_pos == 0 ? screenPos_max - 1 : cursor_pos - 1;

    if (isFieldStart(tempCursorPos))
        return;

    // Backspace one character
    setCursor(tempCursorPos);
}

/**
 * @brief   DisplayScreen::setCursorColour - set the cursor colour
 * @param   inherit - whether the cursor inherits the underlying character colour
 *
 * @details setCursorColour is called when the user changes the way the colour of the cursor is
 *          chosen. The default is for the cursor to be shown with the the colour of the Cell on
 *          which the cursor is placed, but it can be changed to be a static grey colour.
 */
void DisplayScreen::setCursorColour(bool inherit)
{
    cursorColour = inherit;
    if (inherit)
    {
        cursor.setBrush(palette->colour(cells[cursor.data(0).toInt()].getColour()));
    }
    else
    {
        cursor.setBrush(QBrush(QColor(0xBB, 0xBB, 0xBB)));
    }
    cursor.show();
}


/**
 * @brief   DisplayScreen::toggleRuler - toggle the ruler on or off
 *
 * @details Called when the user switches the ruler on or off.
 */
void DisplayScreen::toggleRuler()
{
    // Invert ruler
    rulerOn = !rulerOn;

    setRuler();
}

/**
 * @brief   DisplayScreen::rulerMode - display/hide the ruler
 * @param   on - whether ruler is shown or not
 *
 * @details Called when Settings changes ruler to on or off.
 */
void DisplayScreen::rulerMode(bool on)
{
    rulerOn = on;
    setRuler();
}

/**
 * @brief  DisplayScreen::setRulerStyle - change ruler style
 * @param  rulerStyle - ruler style
 *
 *        rulerStyle | Description
 *        ---------- | -----------
 *          0        | Crosshair
 *          1        | Vertical
 *          2        | Horizontal
 *        other      | Off
 */
void DisplayScreen::setRulerStyle(Q3270::RulerStyle rulerStyle)
{
    this->ruler = rulerStyle;
    setRuler();
}

/**
 * @brief   DisplayScreen::setRuler - set the ruler style and redraw it in case it needs to move
 *
 * @details Called by several other routines when the ruler needs to be changed or the cursor has moved.
 */
void DisplayScreen::setRuler()
{
    if (rulerOn)
    {

        switch(ruler)
        {
            case Q3270::CrossHair:
                crosshair_X.show();
                crosshair_Y.show();
                break;
            case Q3270::Vertical:
                crosshair_X.hide();
                crosshair_Y.show();
                break;
            case Q3270::Horizontal:
                crosshair_X.show();
                crosshair_Y.hide();
        }
        int cursor_y = (cursor_pos / screen_x);
        int cursor_x = cursor_pos - (cursor_y * screen_x);
        crosshair_X.setPos((qreal) cursor_x * gridSize_X, 0);
        crosshair_Y.setPos(0 , (qreal) (cursor_y + 1) * gridSize_Y);
    }
    else
    {
        crosshair_X.hide();
        crosshair_Y.hide();
    }
}
