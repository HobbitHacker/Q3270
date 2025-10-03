/*
 * Q3270 Terminal Emulator
 *
 * Copyright (c) 2020–2025 Andy Styles
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is part of Q3270.
 * See the LICENSE file in the project root for full license information.
 */

#include "CodePage.h"

/**
 * @brief   CodePage::CodePage - Code page representation.
 *
 * @details CodePage contains the various codepages used by 3270 displays. Built
 *          in code pages are currently 037 (US), 285 (UK) and 310 (Graphics).
 *
 *          Each code page is represented by 256 Unicode characters which represent the
 *          EBCDIC code points from 0x00 to 0xFF in Unicode form, and a corresponding
 *          256 bytes held as EBCDIC characters, representing the ASCII (Latin 1) characters
 *          for those positions in EBCDIC.
 *
 *          The default code page at 'power on' is 037.
 */
CodePage::CodePage()
{
    // Set default to IBM--037
    setCodePage("IBM-037");
}

/**
 * @brief   CodePage::getUnicodeChar - return the Unicode character from the EBCDIC character
 * @param   ebcdic - the character in EBCDIC
 * @return  The Unicode equivalent of the EBCDIC character
 *
 * @details getUnicodeChar returns the Unicode equivalent of the EBCDIC character passed.
 */
QString CodePage::getUnicodeChar(uchar ebcdic) const
{
    return cpList[currentCodePage].fromEBCDIC[ebcdic];
}

/**
 * @brief   CodePage::getUnicodeGraphicChar - return the Unicode equivalent of the EBCIDC character
 * @param   ebcdic - the Graphic Escape EBCDIC character
 * @return  The Unicode equivalent of the EBCDIC character
 *
 * @details getUnicodeGraphicChar returns the Unicode character for the EBCIDC "Graphic Escape"
 *          character passed. This is simply the standard EBCDIC character, but from the 310 codepage.
 */
QString CodePage::getUnicodeGraphicChar(uchar ebcdic) const
{
    // GE characters are from the first code page regardless of other codepages; hard code the return source
    return cpList[0].fromEBCDIC[ebcdic];
}

/**
 * @brief   CodePage::getEBCDIC
 * @param   ascii - the ASCII character for which the EBCDIC equivalent is to be returned
 * @return  The EBCDIC character for the passed ASCII equivalent
 *
 * @details Return the EBCDIC character for the ASCII one passed.
 */
uchar CodePage::getEBCDIC(uchar ascii) const
{
    return cpList[currentCodePage].toEBCDIC[ascii];
}

/**
 * @brief   CodePage::setCodePage - set the current codepage
 * @param   codepage - the codepage by name to be set.
 *
 * @details setCodePage is used when the user chooses a different code page, either when loading
 *          a new session setting or manually changing it in the Preferences dialog.
 */
void CodePage::setCodePage(QString codepage)
{
    for(int i = 0; i < CODEPAGE_COUNT; i++)
    {
        if (cpList[i].displayName == codepage)
        {
            currentCodePage = i;
            return;
        }
    }
}

/**
 * @brief   CodePage::getCodePageList - return a QStringList of the available codepages
 * @return  A list of the available code pages
 *
 * @details getCodePageList is used in the Preferences dialog to provide a drop-down list of
 *          codepages that the user can choose from.
 *
 *          Codepages can be hidden from the user's view by setting the 'selectable' flag in the
 *          structure to false.
 */
const QStringList CodePage::getCodePageList() const
{
    QStringList cl;

    for(int i = 0; i < CODEPAGE_COUNT; i++)
    {
        if (cpList[i].selectable)
        {
            cl.append(cpList[i].displayName);
        }
    }

    return cl;
}
