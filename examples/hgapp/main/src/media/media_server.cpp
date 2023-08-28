#include "media_server.h"

#include "log.h"
#include "hg_utils.h"

#define TRACK_RUN

MediaServer::MediaServer()
    : m_topicStatus(false)
    , m_recordStatus(false)
    , m_recordON(false)
    , m_recordOFF(false)
    , m_trackDebug(false)
    , m_mpViDev("/dev/video0")
    , m_mpWidth(1920)
    , m_mpHeight(1080)
    , m_mpViChn(1)
    , m_mpRgaChn720p(0)
    , m_mpVencRtChn(0)
    , m_mpVencRecordChn(1)
    , m_mpVencMjpegChn(2)
    , m_spViDev("/dev/video1")
    , m_spWidth(1048)
    , m_spHeight(780)
    , m_spViChn(0)
    , m_spRgaTrackChn(1)
    , m_spRgaDetectChn(2)
{
}

MediaServer::~MediaServer()
{
    DestoryMpFlows();
    DestorySpFlows();
}

int MediaServer::init()
{
    MPI_SYS_Init();

    check_dir_exist("/block/photo");
    check_dir_exist("/block/video");


    // mp 1080p
    // vi(1920x1080)->venc(1920x1080)->mp4
    //              ->venc(1920x1080)->jpg
    //              ->rga(1280x720)->venc(1280x720)->rtsp
    CreateMpFlows();

    // sp 720p
    // vi(1048x780)->rga->track
    //             ->rga->detect 
    CreateSpFlows();

    return 0;
}

void MediaServer::start()
{
    m_topicStatus = true;
    m_topicSubThread = std::thread(std::bind(&MediaServer::TopicSubThread, this));

    m_recordStatus = true;
    m_recordThread = std::thread(std::bind(&MediaServer::RecordThread, this));

#ifdef TRACK_DEBUG
    m_trackDebug = true;
    m_trackDebugThread = std::thread(std::bind(&MediaServer::RtspThread2, this));
    m_rtspServer2.setBufferAllocCallback(std::bind(&MediaServer::RtspGetBufCallback2, this, std::placeholders::_1));
    m_rtspServer2.setBufferFreeCallback(std::bind(&MediaServer::RtspReleaseBufCallback2, this, std::placeholders::_1));
    m_rtspServer2.start();
#else
    m_rtspServer.setBufferAllocCallback(std::bind(&MediaServer::RtspGetBufCallback, this, std::placeholders::_1));
    m_rtspServer.setBufferFreeCallback(std::bind(&MediaServer::RtspReleaseBufCallback, this, std::placeholders::_1));
    m_rtspServer.start();
#endif
}

void MediaServer::stop()
{
    m_topicStatus = false;
    m_topicSubThread.join();

    m_recordStatus = false; 
    m_recordThread.join();

#ifdef TRACK_DEBUG
    m_trackDebug = false;
    m_rtspServer2.stop();
#else
    m_rtspServer.stop();
#endif
}

void MediaServer::TopicSubThread()
{
    uint64_t media_ctrl_sub = 0;
    uint64_t drone_id_sub = 0;
	
    if (topic_subscribe_auto(TOPIC_ID(media_ctrl), &media_ctrl_sub, (char*)"media", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(media_ctrl)");
    }
    if (topic_subscribe_auto(TOPIC_ID(drone_id), &drone_id_sub, (char*)"drone_id", 10) == -1) {
        HGLOG_ERROR("can not subscribe TOPIC_ID(drone_id)");
    }

	while(m_topicStatus) 
    {
        if (topic_check(&media_ctrl_sub, 0) == 0) {
            media_ctrl_t msg;
			topic_copy(TOPIC_ID(media_ctrl), &msg);

            mediaCtrl(&msg);
		}
        if (topic_check(&drone_id_sub, 0) == 0) {
            drone_id_t msg;
			topic_copy(TOPIC_ID(drone_id), &msg);

            m_droneId = msg.id;
		}

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void MediaServer::mediaCtrl(media_ctrl_t *msg)
{
    int ret = -1;
    switch(msg->cmd) {
        case MAV_CMD_TAKEPHOTO: {
            MPI_VENC_StartRecvFrame(m_mpVencMjpegChn, 1/*recvPicNum*/);
            sendAck2Msg(MAV_CMD_TAKEPHOTO, 0, CMD_OK);
        } break;
        case MAV_CMD_RECORD: {
            if (msg->param[0] == 0) {
                if (m_recordON == false) {
                    m_recordON = true;
                } else {
                    sendAck2Msg(MAV_CMD_RECORD, 0, CMD_REPEAT);
                }
            } else if (msg->param[0] == 1) {
                if (m_recordON == true) {
                    m_recordOFF = true;
                }
            }
        } break;
        case MAV_CMD_FILELIST: {
            if (msg->param[0] == 0) { // get photo list
                ret = get_file_lists("/block/photo/", "/block/photo_lists.json");
                if (ret == -1) {
                    HGLOG_ERROR("[media_server] get photo list json err.");
                    sendAck2Msg(MAV_CMD_FILELIST, 0, CMD_ERR);
                } else {
                    HGLOG_INFO("[media_server] get photo list json ok.");
                    sendAck2Msg(MAV_CMD_FILELIST, 0, CMD_OK);
                }
            } else if (msg->param[0] == 1) { // get video list
                ret = get_file_lists("/block/video/", "/block/video_lists.json");
                if (ret == -1) {
                    HGLOG_ERROR("[media_server] get video list json err.");
                    sendAck2Msg(MAV_CMD_FILELIST, 1, CMD_ERR);
                } else {
                    HGLOG_INFO("[media_server] get video list json ok.");
                    sendAck2Msg(MAV_CMD_FILELIST, 1, CMD_OK);
                }
            }
        } break;
        default:
            break;
    }
}

void MediaServer::sendAck2Msg(uint8_t cmd, int type, uint8_t result)
{
    hgprotocol_lack_t msg = {0};
    msg.id = m_droneId;
    msg.cmd = cmd;
    msg.result = result;
    msg.param[0] = type;
    topic_publish(TOPIC_ID(drone_ack), &msg);
}

HGMEDIA_BUFFER MediaServer::RtspGetBufCallback(void *ptr)
{
    HGMEDIA_BUFFER mb = MPI_SYS_GetMediaBuffer(HGID_VENC, m_mpVencRtChn, -1);
    if (mb) {
        struct H264Attr* attr = (struct H264Attr*)ptr;
        attr->data = MPI_MB_GetPtr(mb);
        attr->size = MPI_MB_GetSize(mb);
        attr->timestamp = MPI_MB_GetTimestamp(mb);
        return mb;
    }
    
    return NULL;
}

void MediaServer::RtspReleaseBufCallback(HGMEDIA_BUFFER mb)
{
    MPI_MB_ReleaseBuffer(mb);
}

void MjpegCallback(HGMEDIA_BUFFER mb) {
    static uint32_t jpeg_id = 0;
    printf("Get JPEG packet[%d]:ptr:%p, fd:%d, size:%d, mode:%d, channel:%d, "
            "timestamp:%ld\n",
            jpeg_id, MPI_MB_GetPtr(mb), MPI_MB_GetFD(mb),
            MPI_MB_GetSize(mb), MPI_MB_GetModeID(mb),
            MPI_MB_GetChannelID(mb), MPI_MB_GetTimestamp(mb));

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char jpeg_path[128] = {0};
    snprintf(jpeg_path, 128, "/block/photo/IMG_%d%02d%02d_%02d%02d%02d_%d.jpg", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, jpeg_id);

    FILE *file = fopen(jpeg_path, "w");
    if (file) {
        fwrite(MPI_MB_GetPtr(mb), 1, MPI_MB_GetSize(mb), file);
        fclose(file);
        //MediaServer::sendAck2Msg(MAV_CMD_TAKEPHOTO, 0, CMD_OK);
    } else {
        HGLOG_ERROR("take_pictures_cb open {} err.", jpeg_path);
        //MediaServer::sendAck2Msg(MAV_CMD_TAKEPHOTO, 0, CMD_ERR);
    }

    MPI_MB_ReleaseBuffer(mb);
    jpeg_id++;
}

int MediaServer::CreateMpFlows()
{
    int ret = -1;
    ret = ViInitChn(m_mpViDev, m_mpViChn, m_mpWidth, m_mpHeight);
    if (ret) {
        HGLOG_ERROR("[MediaServer] create mp viChn failed!!! ret={}", ret);
        return -1;
    } else {
        ret = RgaInitChn(m_mpRgaChn720p, m_mpWidth, m_mpHeight, 1280, 720, HGIMAGE_TYPE_NV12);
        if (ret) {
            HGLOG_ERROR("[MediaServer] create mp rgaChn-720p failed!!! ret={}", ret);
            return -1;
        }

        ret = VencInitChn(m_mpVencRecordChn, m_mpWidth, m_mpHeight);
        if (ret) {
            HGLOG_ERROR("[MediaServer] create mp vencChn-record failed!!! ret={}", ret);
            return -1;
        }
        ret = VencMjpegInitChn(m_mpVencMjpegChn, m_mpWidth, m_mpHeight);
        if (ret) {
            HGLOG_ERROR("[MediaServer] create mp vencChn-mjpeg failed!!! ret={}", ret);
            return -1;
        }
        ret = VencRtInitChn(m_mpVencRtChn, 1280, 720);
        if (ret) {
            HGLOG_ERROR("[MediaServer] create mp vencChn-rt failed!!! ret={}", ret);
            //return 0;
        }

        // Put the jpeg encoder to sleep.
        MPI_VENC_StartRecvFrame(m_mpVencMjpegChn, 0/*recvPicNum*/);

        HGMPP_CHN_S stEncChn;
        stEncChn.modId = HGID_VENC;
        stEncChn.chnId = m_mpVencMjpegChn;
        ret = MPI_SYS_RegisterOutCb(&stEncChn, MjpegCallback);
        if (ret) {
            HGLOG_ERROR("Register Output callback failed! ret={}", ret);
            return -1;
        }

        ret = MpiSysBind(HGID_VI, m_mpViChn, HGID_VPSS, m_mpRgaChn720p);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Bind mp vi and rga-720pp failed!!! ret={}", ret);
            return -1;
        }

        ret = MpiSysBind(HGID_VI, m_mpViChn, HGID_VENC, m_mpVencRecordChn);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Bind mp vi and venc-record failed!!! ret={}", ret);
            return -1;
        }
        ret = MpiSysBind(HGID_VI, m_mpViChn, HGID_VENC, m_mpVencMjpegChn);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Bind mp vi and venc-mjpeg failed!!! ret={}", ret);
            return -1;
        }
        ret = MpiSysBind(HGID_VPSS, m_mpRgaChn720p, HGID_VENC, m_mpVencRtChn);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Bind mp rga-720p and venc-rt failed!!! ret={}", ret);
            //return -1;
        }
    }

    return 0;
}

int MediaServer::DestoryMpFlows()
{
    MpiSysUnBind(HGID_VPSS, m_mpRgaChn720p, HGID_VENC, m_mpVencRtChn);
    MpiSysUnBind(HGID_VI, m_mpViChn, HGID_VENC, m_mpVencRecordChn);
    MpiSysUnBind(HGID_VI, m_mpViChn, HGID_VENC, m_mpVencMjpegChn);
    MpiSysUnBind(HGID_VI, m_mpViChn, HGID_VPSS, m_mpRgaChn720p);

    MPI_VENC_DestoryChn(m_mpVencRtChn);
    MPI_VENC_DestoryChn(m_mpVencRecordChn);
    MPI_VENC_DestoryChn(m_mpVencMjpegChn);
    MPI_VPSS_DestoryChn(m_mpRgaChn720p);
    MPI_VI_DestoryChn(m_mpViChn);

    return 0;
}

int MediaServer::CreateSpFlows()
{
    int ret = -1;   
    ret = ViInitChn(m_spViDev, m_spViChn, m_spWidth, m_spHeight);
    if (ret) {
        HGLOG_ERROR("[MediaServer] Create sp vi failed!!! ret={}", ret);
        //return -1;
    } else {

        #ifdef TRACK_RUN
        #if USE_NCNN_TRACK  
        ret = RgaInitChn(m_spRgaTrackChn, m_spWidth, m_spHeight, m_spWidth, m_spHeight, HGIMAGE_TYPE_RGB888);
        #else
        ret = RgaInitChn(m_spRgaTrackChn, m_spWidth, m_spHeight, m_spWidth, m_spHeight, HGIMAGE_TYPE_NV12);
        #endif
        if (ret) {
            HGLOG_ERROR("[MediaServer] Create rga-track falied!!! ret={}", ret);
            //return -1;
        }
        ret = MpiSysBind(HGID_VI, m_spViChn, HGID_VPSS, m_spRgaTrackChn);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Bind sp vi and rga-track failed!!! ret={}", ret);
            //return -1;
        }
        #endif

        #ifdef TRACK_DEBUG
        ret = RgaInitChn(8, m_spWidth, m_spHeight, m_spWidth, m_spHeight, HGIMAGE_TYPE_NV12);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Create rga-track-rtsp falied!!! ret={}", ret);
            //return -1;
        }
        ret = MpiSysBind(HGID_VI, m_spViChn, HGID_VPSS, 8);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Bind sp vi and rga-track-rtsp failed!!! ret={}", ret);
            //return -1;
        }
        ret = VencInitChn(8, m_spWidth, m_spHeight);
        if (ret) {
            HGLOG_ERROR("[MediaServer] create mp vencChn-track-rtsp failed!!! ret={}", ret);
            return -1;
        }
        #endif

        #ifdef NPU_RUN
        ret = RgaInitChn(m_spRgaDetectChn, m_spWidth, m_spHeight, 640, 360, IMAGE_TYPE_BGR888); //IMAGE_TYPE_RGB888
        if (ret) {
            HGLOG_ERROR("[MediaServer] Create rga-detect falied!!! ret={}", ret);
            //return -1;
        }
        ret = MpiSysBind(HGID_VI, m_spViChn, HGID_VPSS, m_spRgaDetectChn);
        if (ret) {
            HGLOG_ERROR("[MediaServer] Bind sp vi and rga-detect failed!!! ret={}", ret);
            //return -1;
        }
        #endif
    }

    return ret;
}

int MediaServer::DestorySpFlows()
{

#ifdef TRACK_RUN
    MpiSysUnBind(HGID_VI, m_spViChn, HGID_VPSS, m_spRgaTrackChn);
    MPI_VPSS_DestoryChn(m_spRgaTrackChn);
#endif

#ifdef TRACK_DEBUG
    MpiSysUnBind(HGID_VI, m_spViChn, HGID_VPSS, 8);
    MPI_VENC_DestoryChn(8);
    MPI_VPSS_DestoryChn(8);
#endif

#ifdef NPU_RUN
    MpiSysUnBind(HGID_VI, m_spViChn, HGID_VPSS, m_spRgaDetectChn);
    MPI_VPSS_DestoryChn(m_spRgaDetectChn);
#endif
    
    MPI_VI_DestoryChn(m_spViChn);

    return 0;
}

int MediaServer::ViInitChn(char* dev, int chn, int width, int height)
{
    int ret = -1;
    HGVI_CONFIG_S stViAttr;
    stViAttr.dev = dev;
    stViAttr.chn = chn;
    stViAttr.width = width;
    stViAttr.height = height;
    ret = MPI_VI_CreateChn(stViAttr);

    return ret;
}

int MediaServer::RgaInitChn(int chn, int srcW, int srcH, int dstW, int dstH, HGIMAGE_TYPE_E dstType)
{
    int ret = -1;
    HGVPSS_CONFIG_S stVpssAttr = {0};
    stVpssAttr.chn = chn;
    stVpssAttr.srcType = HGIMAGE_TYPE_NV12;
    stVpssAttr.srcWidth = srcW;
    stVpssAttr.srcHeight = srcH;
    stVpssAttr.srcHorStride = srcW;
    stVpssAttr.srcVirStride = srcH;
    stVpssAttr.dstType = dstType;
    stVpssAttr.dstWidth = dstW;
    stVpssAttr.dstHeight = dstH;
    stVpssAttr.dstHorStride = dstW;
    stVpssAttr.dstVirStride = dstH;
    ret = MPI_VPSS_CreateChn(stVpssAttr);

    return ret;
}

// int MediaServer::RgaInitChnCrop(int chn, int srcW, int srcH, int dstW, int dstH, HGIMAGE_TYPE_E dstType)
// {
//     int ret = -1;
//     HGVPSS_CONFIG_S stVpssAttr = {0};
//     stVpssAttr.chn = chn;
//     stVpssAttr.srcType = HGIMAGE_TYPE_NV12;
//     stVpssAttr.srcY = 190;
//     stVpssAttr.srcWidth = dstW;
//     stVpssAttr.srcHeight = dstH;
//     stVpssAttr.srcHorStride = srcW;
//     stVpssAttr.srcVirStride = srcH;
//     stVpssAttr.dstType = dstType;
//     stVpssAttr.dstWidth = dstW;
//     stVpssAttr.dstHeight = dstH;
//     stVpssAttr.dstHorStride = dstW;
//     stVpssAttr.dstVirStride = dstH;
//     ret = MPI_VPSS_CreateChn(stVpssAttr);

//     return ret;
// }

int MediaServer::VencInitChn(int chn, int width, int height)
{
    int ret = -1;
    HGVENC_CONFIG_S stVencAttr;
    stVencAttr.chn = chn;
    stVencAttr.width = width;
    stVencAttr.height = height;
    stVencAttr.virWidth = width;
    stVencAttr.virHeight = height;
    stVencAttr.profile = 72;
    stVencAttr.gop = 30;
    stVencAttr.bitRate = 4000000;
    stVencAttr.qpSet = 0;
    stVencAttr.startQp = 30;
    stVencAttr.stepQp = 6;
    stVencAttr.maxIQp = 51;
    stVencAttr.minIQp = 20;
    stVencAttr.maxQp = 45;
    stVencAttr.minQp = 25;
    ret = MPI_VENC_CreateChn(stVencAttr);
  
    return ret; 
}

int MediaServer::VencMjpegInitChn(int chn, int width, int height)
{
    int ret = 0;
    ret = MPI_VENC_CreateMjpegChn(chn, width, height);

    return ret; 
}

int MediaServer::VencRtInitChn(int chn, int width, int height)
{
    int ret = 0;
    HGVENC_CONFIG_S stVencAttr;
    stVencAttr.chn = chn;
    stVencAttr.width = width;
    stVencAttr.height = height;
    stVencAttr.virWidth = width;
    stVencAttr.virHeight = height;
    stVencAttr.profile = 72;
    stVencAttr.gop = 30;
    stVencAttr.bitRate = 4000000;
    stVencAttr.qpSet = 0;
    stVencAttr.startQp = 30;
    stVencAttr.stepQp = 6;
    stVencAttr.maxIQp = 51;
    stVencAttr.minIQp = 20;
    stVencAttr.maxQp = 45;
    stVencAttr.minQp = 25;
    ret = MPI_VENC_CreateChn(stVencAttr);

    return ret; 
}

int MediaServer::MpiSysBind(HGMOD_ID_E source, int chn_source, HGMOD_ID_E sink, int chn_sink)
{
    int ret = 0;
    HGMPP_CHN_S stSourceChn;
    stSourceChn.modId = source;
    stSourceChn.devId = 0;
    stSourceChn.chnId = chn_source;
    HGMPP_CHN_S stSinkChn;
    stSinkChn.modId = sink;
    stSinkChn.devId = 0;
    stSinkChn.chnId = chn_sink; 
    ret = MPI_SYS_Bind(&stSourceChn, &stSinkChn);

    return ret;
}

int MediaServer::MpiSysUnBind(HGMOD_ID_E source, int chn_source, HGMOD_ID_E sink, int chn_sink)
{
    int ret = 0;
    HGMPP_CHN_S stSourceChn;
    stSourceChn.modId = source;
    stSourceChn.devId = 0;
    stSourceChn.chnId = chn_source;
    HGMPP_CHN_S stSinkChn;
    stSinkChn.modId = sink;
    stSinkChn.devId = 0;
    stSinkChn.chnId = chn_sink; 
    ret = MPI_SYS_UnBind(&stSourceChn, &stSinkChn);
  
    return ret;
}

void MediaServer::RecordThread() 
{
    HGMEDIA_BUFFER mb = NULL;
    FILE* fp_mp4 = NULL;
    bool mp4file_create_switch = true;
    int cnt = 0;
    char vid_path[128] = {0};

    while (m_recordStatus) {
        mb = MPI_SYS_GetMediaBuffer(HGID_VENC, m_mpVencRecordChn, -1);
        //mb = MPI_SYS_GetMediaBuffer(HGID_VENC, m_mpVencRtChn, 3000);
        if (!mb) {
            HGLOG_ERROR("[media_server] record get null buffer!\n");
            break;
        }

        // printf("Get Venc packet[%d]:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
        //     "timestamp:%lld\n",
        //     0, MPI_MB_GetPtr(mb), /*MPI_MB_GetFD(mb)*/0,
        //     MPI_MB_GetSize(mb), MPI_MB_GetModeID(mb),
        //     MPI_MB_GetChannelID(mb), MPI_MB_GetTimestamp(mb));        

        if (m_recordON) {
            if (mp4file_create_switch) {
                HGLOG_INFO("[media_server] record start.");
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                snprintf(vid_path, 128, "/block/video/VID_%d%02d%02d_%02d%02d%02d_%d.mp4", tm.tm_year + 1900,
                                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, cnt);
                fp_mp4 = fopen(vid_path, "wb+");
                if (!fp_mp4) {
                    HGLOG_ERROR("[media_server] record: Open %s failed!", vid_path);
                } else {
                    mp4_muxer_create(fp_mp4, m_mpWidth, m_mpHeight);
                    mp4file_create_switch = false;
                    sendAck2Msg(MAV_CMD_RECORD, 0, CMD_OK);
                    cnt ++;
                }
            }
            mp4_muxer_write(MPI_MB_GetPtr(mb), MPI_MB_GetSize(mb));
        }
        if (m_recordOFF && mp4file_create_switch == false) {
            HGLOG_INFO("[media_server] record stop.");
            mp4file_create_switch = true;
            m_recordON = false;
            m_recordOFF  = false;
            mp4_muxer_destroy();
            if (fp_mp4)
                fclose(fp_mp4);
            sendAck2Msg(MAV_CMD_RECORD, 1, CMD_OK);
        } 

        MPI_MB_ReleaseBuffer(mb);
    }
}

#ifdef TRACK_DEBUG
HGMEDIA_BUFFER MediaServer::RtspGetBufCallback2(void *ptr)
{
    HGMEDIA_BUFFER mb = MPI_SYS_GetMediaBuffer(HGID_VENC, 8, -1);
    if (mb) {
        struct H264Attr* attr = (struct H264Attr*)ptr;
        attr->data = MPI_MB_GetPtr(mb);
        attr->size = MPI_MB_GetSize(mb);
        attr->timestamp = MPI_MB_GetTimestamp(mb);
        //printf("--------------- %d\n", attr->size);
        return mb;
    }
    
    return NULL;
}

void MediaServer::RtspReleaseBufCallback2(HGMEDIA_BUFFER mb)
{
    MPI_MB_ReleaseBuffer(mb);
}

extern std::mutex mutex_rect;
extern int track_rect_x,track_rect_y,track_rect_w,track_rect_h;
void MediaServer::RtspThread2()
{
    HGMEDIA_BUFFER mb = NULL;
    int rect_x,rect_y,rect_w,rect_h;
    while (m_trackDebug) {
        mb = MPI_SYS_GetMediaBuffer(HGID_VPSS, 8, -1);
        if (!mb) {
            HGLOG_ERROR("[media_server] track vpss get null buffer!\n");
            continue;
        }
        //printf("77777777777777 %d\n", MPI_MB_GetSize(mb));
        void *ptr = MPI_MB_GetPtr(mb);
        {
            std::lock_guard<std::mutex> slock(mutex_rect);
            rect_x=track_rect_x;
            rect_y=track_rect_y;
            rect_w=track_rect_w;
            rect_h=track_rect_h;
        }
        drawBoundingBox((unsigned char*)ptr, 1048, 780, rect_x,rect_y,rect_w,rect_h);

        MPI_SYS_SendMediaBuffer(HGID_VENC, 8, mb);

        MPI_MB_ReleaseBuffer(mb);
    }
}

void MediaServer::drawBoundingBox(unsigned char* nv12Data, int width, int height, int x, int y, int boxWidth, int boxHeight) {
    x=x/2*2;
    y=y/2*2;
    boxWidth=boxWidth/2*2;
    boxHeight=boxHeight/2*2;
    unsigned char u=127;
    unsigned char v=255;
    for(int c=x;c<x+boxWidth;c++){
        nv12Data[y*width+c]=255;
        nv12Data[(y+boxHeight)*width+c]=255;
    }
    for(int r=y;r<y+boxHeight;r++){
        nv12Data[r*width+x]=255;
        nv12Data[r*width+x+boxWidth]=255;
    }
    int uvPlaneOffset = width * height;
    for(int i=x;i<boxWidth+x;i=i+2){
        nv12Data[uvPlaneOffset+y/2*width+i]=u;//u
        nv12Data[uvPlaneOffset+y/2*width+i+1]=v;//v
        
        nv12Data[uvPlaneOffset+(y+boxHeight)/2*width+i]=u;//u
        nv12Data[uvPlaneOffset+(y+boxHeight)/2*width+i+1]=v;//v
    }
    for(int i=y;i<boxHeight+y;i=i+2){
        nv12Data[uvPlaneOffset+i/2*width+x]=u;//u
        nv12Data[uvPlaneOffset+i/2*width+x+1]=v;//v
     
        nv12Data[uvPlaneOffset+i/2*width+x+boxWidth]=u;//u
        nv12Data[uvPlaneOffset+i/2*width+x+boxWidth+1]=v;//v
    }

    // int uvWidth = width / 2;
    // int uvHeight = height / 2;
    // int startX = x/2;
    // int startY = y/2;
    // boxWidth = boxWidth/2;
    // boxHeight = boxHeight/2;
    // // 计算UV平面的起始位置
    // int uvPlaneOffset = width * height;
    // int vOffset=uvPlaneOffset+uvWidth*uvHeight;
    // for(int i=startX;i<boxWidth;i++){
    //     nv12Data[uvPlaneOffset+startY*uvWidth+i]=50;
    //     nv12Data[uvPlaneOffset+(startY+boxHeight)*uvWidth+i]=50;
    // }
    // for(int i=startY;i<boxHeight;i++){
    //     nv12Data[vOffset+i*uvWidth+startX]=50;
    //     nv12Data[vOffset+i*uvWidth+startX+boxWidth]=50;
    // }

    // // 计算UV平面的起始位置
    // int uvPlaneOffset = width * height;

    // // 计算UV分量的起始位置
    // int uvStartOffset = uvPlaneOffset + (y / 2) * width;

    // // 计算UV分量的宽度和高度（每个UV像素包含2个亮度像素）
    // int uvWidth = boxWidth / 2;
    // int uvHeight = boxHeight / 2;

    // // 绘制UV分量上的框线，将对应位置的像素设置为255，表示白色
    // for (int i = 0; i < uvHeight; i++) {
    //     int uvLineOffset = uvStartOffset + (i * width);

    //     // 左边框线
    //     nv12Data[uvLineOffset +x ] = 0;
    //     // nv12Data[uvLineOffset +x+1 ] = 127;

    //     // 右边框线
    //     nv12Data[uvLineOffset + x + boxWidth - 1] = 20;
    //     // nv12Data[uvLineOffset + x+1 + boxWidth - 1] = 127;
    // }

    // for (int i = 0; i < uvWidth; i++) {
    //     // 左边框线
    //     nv12Data[uvStartOffset +x +i*2 ] = 40;
    //     // nv12Data[uvStartOffset +x +i*2+1 ] = 127;

    //     // 右边框线
    //     nv12Data[uvStartOffset + x + i*2 + (uvHeight - 1)*width] = 60;
    //     // nv12Data[uvStartOffset + x + i*2+1 + (uvHeight - 1)*width] = 127;
    // }
}
#endif