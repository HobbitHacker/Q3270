#include "DisplayData.h"

DisplayData::DisplayData(int size)
{
    display = (unsigned char *)malloc(size);
    maxSize = size;
}

DisplayData::~DisplayData()
{
    free(display);
}

void DisplayData::clear()
{
    memset(display, 0, maxSize);
}

void DisplayData::setChar(int pos, unsigned char c)
{
    *(display+pos) = c;
}

unsigned char DisplayData::getChar(int pos)
{
    return *(display+pos);
}
