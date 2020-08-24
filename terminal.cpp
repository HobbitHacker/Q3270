#include "Terminal.h"

Terminal::Terminal(QString type, int termX, int termY)
{
    setType(type);
    if (type == 4)
    {
        terms[4].x = termX;
        terms[4].y = termY;
    }
}

void Terminal::setType(QString type)
{
    for (int i = 0; i < 5; i++)
    {
        if (type == terms->term[i])
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

int Terminal::width()
{
    return terms[termType].x;
}

int Terminal::height()
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

char * Terminal::name()
{
    return terms[termType].term.toLatin1().data();
}
