#ifndef DISPLAYDATA_H
#define DISPLAYDATA_H

#include <stdlib.h>
#include <string.h>

class DisplayData
{
    public:
        DisplayData(int size);
        ~DisplayData();

        void setChar(int pos, unsigned char c);
        unsigned char getChar(int pos);
        void clear();

    private:

        unsigned char *display;
        int maxSize;

};

#endif // DISPLAYDATA_H
