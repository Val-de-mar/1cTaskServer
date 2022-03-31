#ifndef TASK_DYNAMICBUFFER_H
#define TASK_DYNAMICBUFFER_H

#include "PrimitiveResult.h"
#include <string.h>

const static size_t BUF_MAX = 4096;


typedef enum ReadResultStruct {
    LineRead,
    WaitingRead,
    EndOfFileRead,
    ErrorRead,
} ReadResult;

typedef struct DynamicBufferStruct {
    char *data;
    size_t capacity;
    size_t size;    //size <= capacity - 1 //always
} DynamicBuffer;


//char *strnchr(char *str, size_t n, char c);

PrimitiveResult bufferExtendIfRequired(DynamicBuffer *buffer, size_t up_to);

PrimitiveResult bufferPushStr(DynamicBuffer *buffer, char *str, size_t n);

PrimitiveResult bufferInit(DynamicBuffer *buffer);

void bufferDestroy(DynamicBuffer *buffer);

void bufferCut(DynamicBuffer *buffer, size_t from);


ReadResult readLine(int fd, DynamicBuffer *buffer);

#endif //TASK_DYNAMICBUFFER_H
