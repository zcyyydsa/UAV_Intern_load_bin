#ifndef __HGRTSP_TYPES_H_
#define __HGRTSP_TYPES_H_

#include <functional>

#include <stdint.h>

struct H264Attr
{
    void* data;
    int size;
    uint32_t timestamp;
};

typedef std::function<void* (void* param)> BufferAllocCallback;
typedef std::function<void (void* buf)> BufferFreeCallback;

#endif