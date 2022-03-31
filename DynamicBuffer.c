//
// Created by val-de-mar on 31.03.2022.
//

#include "DynamicBuffer.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

char *strnchr(char *str, size_t n, char c) {
    for (char *i = str; i < str + n; ++i) {
        if (*i == c) {
            return i;
        }
    }
    return NULL;
}

ReadResult readLine(int fd, DynamicBuffer *buffer) {
    if (strnchr(buffer->data, buffer->size, '\n') != NULL) {
        return LineRead;
    }

    char buf[BUF_MAX];
    int read_res = read(fd, buf, BUF_MAX);
    while (1) {
        switch (read_res) {
            case -1:
                if (errno == EAGAIN) {
                    return WaitingRead;
                } else {
                    return ErrorRead;
                }
            case 0:
                return EndOfFileRead;
        }
        bufferPushStr(buffer, buf, read_res);
        char *newline = strnchr(buf, BUF_MAX, '\n');
        if (newline != NULL) {
            return LineRead;
        }
        read_res = read(fd, buf, BUF_MAX);
    }
}


PrimitiveResult bufferExtendIfRequired(DynamicBuffer *buffer, size_t up_to) {
    while (buffer->capacity <= buffer->size + up_to) {
        buffer->capacity <<= 1;
    }
    char *new_place = realloc(buffer->data, buffer->capacity);
    if (new_place == NULL) {
        return ErrorResult;
    }
    buffer->data = new_place;
    buffer->data[buffer->capacity - 1] = '\0';
    return OkResult;
}

PrimitiveResult bufferPushStr(DynamicBuffer *buffer, char *str, size_t n) {
    if (bufferExtendIfRequired(buffer, n + buffer->size) == ErrorResult) {
        return ErrorResult;
    }
    memcpy(buffer->data + buffer->size, str, n);
    buffer->size += n;
    return OkResult;
}

PrimitiveResult bufferInit(DynamicBuffer *buffer) {
    buffer->data = malloc(1);
    if (buffer->data == NULL) {
        return ErrorResult;
    }
    buffer->capacity = 1;
    buffer->size = 0;
    return OkResult;
}

void bufferCut(DynamicBuffer *buffer, size_t from) {
    memmove(buffer->data, buffer->data + from, buffer->size - from);
    buffer->size -= from;
}

void bufferDestroy(DynamicBuffer *buffer) {
    free(buffer->data);
    buffer->size = 0;
    buffer->capacity = 0;
}
