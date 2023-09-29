#include "CodePage.h"

CodePage::CodePage()
{
    // Set default to IBM--037
    setCodePage("IBM-037");
}

QString CodePage::getUnicodeChar(uchar ebcdic)
{
    return cpList[currentCodePage].fromEBCDIC[ebcdic];
}

QString CodePage::getUnicodeGraphicChar(uchar ebcdic)
{
    // GE characters are from the first code page; hard code the return source
    return cpList[0].fromEBCDIC[ebcdic];
}

uchar CodePage::getEBCDIC(uchar ascii)
{
    return cpList[currentCodePage].toEBCDIC[ascii];
}

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

QMap<QString, QString> CodePage::getCodePageList()
{
    QMap<QString, QString> cl;

    for(int i = 0; i < CODEPAGE_COUNT; i++)
    {
        if (cpList[i].selectable)
        {
            cl.insert(cpList[i].displayName, cpList[i].cpName);
        }
    }

    return cl;
}
