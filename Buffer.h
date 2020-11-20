#ifndef BUFFER_H
#define BUFFER_H

#include <stdlib.h>
#include <stdio.h>

class Buffer
{
    #define MAX_SIZE 1024000

    public:
        Buffer(int bytes = MAX_SIZE);
        ~Buffer();
        int size();
        void add(char b);
        Buffer *nextByte();
        unsigned char getByte();
        bool moreBytes();
        unsigned char getByte(int offset);
        bool compare(int offset, char *string);
        void reset();
        void restart();
        bool byteEquals(int offset, unsigned char byte);
        bool processing();
        void setProcessing(bool mode);

        const char *address();
        void dump(bool send = false);

    private:

        bool busy;
        unsigned char *buffer;
        unsigned char *bufferEnd;
        unsigned char *next;

};

#endif // BUFFER_H
