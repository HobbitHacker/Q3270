#include "CodePage.h"

CodePage::CodePage(int cp)
{
    currentCodePage = 037;
}

QString CodePage::getUnicodeChar(uchar ebcdic)
{
    return cp037[ebcdic];
}

QString CodePage::getUnicodeGraphicChar(uchar ebcdic)
{
    return cp310[ebcdic];
}

uchar CodePage::getEBCDIC(uchar ascii)
{
    return ASCIItoCP037[ascii];
}

void CodePage::setCodePage(QString codepage)
{

}

QString CodePage::getCodePage()
{
    return "IBM-037";
}
