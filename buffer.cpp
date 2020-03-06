#include "buffer.h"

Buffer::Buffer(int bytes)
{
    buffer = (unsigned char *)malloc(bytes);
    bufferEnd = buffer;
    next = buffer;
    busy = false;
}

Buffer::~Buffer()
{
    free(buffer);
}

const char* Buffer::address()
{
    return (char *)buffer;
}

void Buffer::add(char b)
{
    *bufferEnd++ = b;
}

void Buffer::reset()
{
    bufferEnd = buffer;
    next = buffer;
    busy = false;
}

void Buffer::setProcessing(bool b)
{
    busy = b;
}

bool Buffer::processing()
{
    return busy;
}

int Buffer::size()
{
    return bufferEnd - buffer;
}

Buffer *Buffer::nextByte()
{
    if (++next > (buffer + MAX_SIZE))
    {
        printf("EXCEPTION: beyond buffer MAX_SIZE");
        fflush(stdout);
        return nullptr;
    }

    if (next >= bufferEnd)
    {
        return nullptr;
    }

    return this;
}

unsigned char Buffer::getByte()
{
    return *next;
}

void Buffer::dump()
{
    printf("\n\n---------------- Buffer (size = %d) ------------------", size());
    unsigned char *c = buffer;
    for(int i = 0; i < size(); i++)
    {
        if (i%16 == 0)
        {
            printf("\n%04.4X : ", i);
        }
        printf("%02.2X ", *c++);
    }
    printf("\n---------------- Buffer (size = %d) ------------------\n\n", size());
    fflush(stdout);
}

