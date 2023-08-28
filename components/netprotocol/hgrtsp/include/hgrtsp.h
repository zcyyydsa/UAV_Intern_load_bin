#ifndef __HGRTSP_H_
#define __HGRTSP_H_

#include "hgrtsp_types.h"

namespace hgdf {

class Rtsp;
class RtspServer
{
public:
    RtspServer();
    ~RtspServer();
    void setSdp();
    int start();
    void stop();
    bool getStatus();
    void setBufferAllocCallback(const BufferAllocCallback& cb);
    void setBufferFreeCallback(const BufferFreeCallback& cb);
    
private:
    Rtsp* rtsp_;
};

}

#endif
