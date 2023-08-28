#include <stdio.h>

#include "hgmedia.h"
#include "hgrtsp.h"

#include <pthread.h>

#include <functional>

using namespace hgdf;

#define MP_DEV "/dev/video0"
#define MP_W 1920
#define MP_H 1080
#define MP_VI 0
#define MP_VENC_RECORD 1
#define MP_VENC_MJPEG 2

#define SP_DEV "/dev/video1"
#define SP_W 1280
#define SP_H 720
#define SP_VI 1
#define SP_VENC_RT 0


HGMEDIA_BUFFER get_vencbuf(void *ptr)
{
    HGMEDIA_BUFFER mb = MPI_SYS_GetMediaBuffer(HGID_VENC, SP_VENC_RT, -1);
    if (mb) {
        struct H264Attr* attr = (struct H264Attr*)ptr;
        attr->data = MPI_MB_GetPtr(mb);
        attr->size = MPI_MB_GetSize(mb);
        attr->timestamp = MPI_MB_GetTimestamp(mb);
        //printf("play %d\n", attr->timestamp);
        return mb;
    }
    
    return NULL;
}

void release_vencbuf(HGMEDIA_BUFFER mb)
{
    MPI_MB_ReleaseBuffer(mb);
}

int main()
{
    int ret = -1;
    
    MPI_SYS_Init();
    
    HGVI_CONFIG_S stViAttr;
    stViAttr.dev = SP_DEV;
    stViAttr.chn = SP_VI;
    stViAttr.width = SP_W;
    stViAttr.height = SP_H;
    ret = MPI_VI_CreateChn(stViAttr);
    if (ret != ERR_SYS_OK) {
        printf("MPI_VI_CreateChn err.\n");
    }
    
    HGVENC_CONFIG_S stVencAttr;
    stVencAttr.chn = SP_VENC_RT;
    stVencAttr.width = SP_W;
    stVencAttr.height = SP_H;
    stVencAttr.virWidth = SP_W;
    stVencAttr.virHeight = SP_H;
    stVencAttr.profile = 72;
    stVencAttr.gop = 30;
    stVencAttr.bitRate = 400000;
    stVencAttr.qpSet = 0;
    stVencAttr.startQp = 30;
    stVencAttr.stepQp = 6;
    stVencAttr.maxIQp = 51;
    stVencAttr.minIQp = 20;
    stVencAttr.maxQp = 45;
    stVencAttr.minQp = 25;
    ret = MPI_VENC_CreateChn(stVencAttr);
    if (ret != ERR_SYS_OK) {
        printf("MPI_VENC_CreateChn err.\n");
    }

    HGMPP_CHN_S stSourceChn;
    stSourceChn.modId = HGID_VI;
    stSourceChn.devId = 0;
    stSourceChn.chnId = SP_VI;
    HGMPP_CHN_S stSinkChn;
    stSinkChn.modId = HGID_VENC;
    stSinkChn.devId = 0;
    stSinkChn.chnId = SP_VENC_RT; 
    MPI_SYS_Bind(&stSourceChn, &stSinkChn);

    RtspServer rtspServer;
    rtspServer.setBufferAllocCallback(std::bind(get_vencbuf, std::placeholders::_1));
    rtspServer.setBufferFreeCallback(std::bind(release_vencbuf, std::placeholders::_1));
    rtspServer.start();

    getchar();
    
    rtspServer.stop();

    MPI_SYS_UnBind(&stSourceChn, &stSinkChn);
    MPI_VENC_DestoryChn(stVencAttr.chn);
    MPI_VI_DestoryChn(stViAttr.chn);

    return 0;
}
