#ifndef __MEDIA_SERVER_H_
#define __MEDIA_SERVER_H_

#include <thread>

#include "hgmedia.h"
#include "hgmuxer.h"
#include "hgrtsp.h"
#include "uorb_topic.h"
#include "hg_protocol_types.h"

using namespace hgdf;

#define TRACK_DEBUG

class MediaServer
{
public:
    MediaServer();
    ~MediaServer();

    int init();
    void start();
    void stop();
    void sendAck2Msg(uint8_t cmd, int type, uint8_t result);
private:
    int CreateMpFlows();
    int DestoryMpFlows();

    int CreateSpFlows();
    int DestorySpFlows();

    int ViInitChn(char* dev, int chn, int width, int height);
    int RgaInitChn(int chn, int srcW, int srcH, int dstW, int dstH, HGIMAGE_TYPE_E dstType);
    // int RgaInitChnCrop(int chn, int srcW, int srcH, int dstW, int dstH, HGIMAGE_TYPE_E dstType);
    int VencInitChn(int chn, int width, int height);
    int VencMjpegInitChn(int chn, int width, int height);
    int VencRtInitChn(int chn, int width, int height);

    int MpiSysBind(HGMOD_ID_E source, int chn_source, HGMOD_ID_E sink, int chn_sink);
    int MpiSysUnBind(HGMOD_ID_E source, int chn_source, HGMOD_ID_E sink, int chn_sink);

    HGMEDIA_BUFFER RtspGetBufCallback(void *ptr);
    void RtspReleaseBufCallback(HGMEDIA_BUFFER mb);

#ifdef TRACK_DEBUG
    HGMEDIA_BUFFER RtspGetBufCallback2(void *ptr);
    void RtspReleaseBufCallback2(HGMEDIA_BUFFER mb);
    void drawBoundingBox(unsigned char* nv12Data, int width, int height, int x, int y, int boxWidth, int boxHeight);
    
    void RtspThread2();
    std::thread m_trackDebugThread;
    RtspServer m_rtspServer2;
#endif

    void mediaCtrl(media_ctrl_t *msg);  
private:
    void TopicSubThread();
    void RecordThread();
    //void RtspThread();

private:
    std::thread m_topicSubThread;
    std::thread m_recordThread;
    bool m_topicStatus; 
    bool m_recordStatus;
    bool m_recordON;
    bool m_recordOFF;
    bool m_trackDebug;

    char* m_mpViDev;
    int m_mpWidth;
    int m_mpHeight;
    int m_mpViChn;
    int m_mpRgaChn720p;
    int m_mpVencRtChn;
    int m_mpVencRecordChn;
    int m_mpVencMjpegChn;

    char* m_spViDev;
    int m_spWidth;
    int m_spHeight;
    int m_spViChn;
    int m_spRgaTrackChn;
    int m_spRgaDetectChn;


    RtspServer m_rtspServer;
    uint16_t m_droneId;
};

#endif