#include "Buffer.h"

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

void Buffer::restart()
{
    next = buffer;
}

bool Buffer::byteEquals(int offset, unsigned char byte)
{
    if (offset > size())
    {
        return false;
    }
    if (*(buffer+offset) == byte)
    {
        return true;
    }
    return false;
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

bool Buffer::moreBytes()
{
    if (next >= bufferEnd)
    {
        return false;
    }
    return true;
}

bool Buffer::compare(int offset, char *string)
{
    for (int i = 0; *(string+i) != 0; i++)
    {
        if (!byteEquals(offset + i, *(string+i)))
        {
            return false;
        }
    }
    return true;
}

unsigned char Buffer::getByte(int offset)
{
    if (offset + buffer > bufferEnd)
    {
        printf("EXCEPTION: getByte(%d) exceeded buffer size %d\n", offset, size());
        return 0;
    }
    return *(buffer+offset);
}

unsigned char Buffer::getByte()
{
    return *next;
}

void Buffer::dump(bool send)
{

    if (!send)
    {
        printf("\n\n---------------- Buffer (size = %d) ------------------\n", size());
    }
    else
    {
        printf("\n\nSEND------------- Buffer (size = %d) ------------------\n", size());
    }
    printf("Current Position: %4.4X (%2.2X)", next - buffer, *next);
    unsigned char *c = buffer;
    for(int i = 0; i < size(); i++)
    {
        if (i%32 == 0)
        {
            printf("\n%04.4X : ", i);
        }
        printf("%02.2X", *c++);
    }
    printf("\n---------------- Buffer (size = %d) ------------------\n\n", size());
    fflush(stdout);
}

void Buffer::addBlock(unsigned char *bytes, int length)
{
    for (int i = 0; i < length; i++)
    {
        add(bytes[i]);
    }
}

