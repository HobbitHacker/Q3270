#ifndef TERMINAL_H
#define TERMINAL_H

#include <QString>

class Terminal
{
    public:
        Terminal(QString type = "IBM-3279-2-E", int termX = 0, int termY = 0);

        int width();
        int height();
        char *name();

        void setWidth(int w);
        void setHeight(int h);

        void setType(QString type);
        void setType(int type);
        void setSize(int x, int y);

    private:

        struct termTypes
        {
            QString term;
            int x, y;
        };

        termTypes terms[5] = {
            { "IBM-3279-2-E", 80, 24 },
            { "IBM-3279-3-E", 80, 32 },
            { "IBM-3279-4-E", 80, 43 },
            { "IBM-3279-5-E", 132, 27 },
            { "IBM-DYNAMIC", 0, 0}
        };

        int termType;

};

#endif // TERMINAL_H
