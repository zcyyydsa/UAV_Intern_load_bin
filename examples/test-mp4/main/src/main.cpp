
#include "hgmedia.h"
#include "hgmuxer.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

using namespace hgdf;

int8_t g_viChn = 0;
uint8_t g_vencChn = 0;
uint32_t g_frameCnt = 10;
uint32_t g_width = 1920;
uint32_t g_height = 1080;
char *g_pOutPath = "/tmp/out.mp4";

static bool g_quit = false;
static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    g_quit = true;
}

static char optstr[] = "?:i:o:h";
static const struct option long_options[] = {
    {"input", required_argument, NULL, 'i'},
    {"output", required_argument, NULL, 'o'},
    {"help", no_argument, NULL, 'h'},
    {NULL, 0, NULL, 0},
};

static void print_usage(const char *name)
{
    printf("usage example:\n");
    printf("\t%s "
           "[-i | --input /tmp/1080p_nv12.yuv] "
           "[-o | --output /tmp/out.h264] "
           "[-h | --help] ",
           name);
    printf("\t-i | --input: yuv file, Default:/tmp/1080p_nv12.yuv\n");
    printf("\t-o | --output: out file, Default:/tmp/out.h264\n");
    printf("\t-h | --help: show help\n");
}

static void *GetMediaBuffer(void *arg)
{
    printf("#Start %s thread, arg:%p \n", __func__);
    FILE* fp_mp4 = fopen(g_pOutPath, "wb+");
    if (!fp_mp4)
        printf("ERROR: Open %s failed!\n", g_pOutPath);

    mp4_muxer_create(fp_mp4, g_width, g_height);

    HGMEDIA_BUFFER mb = NULL;
    while (!g_quit)
    {
        mb = MPI_SYS_GetMediaBuffer(HGID_VENC, g_vencChn, -1);
        if (!mb)
        {
            printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
            break;
        }

        printf("Get packet:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
               "timestamp:%lld\n",
               MPI_MB_GetPtr(mb), MPI_MB_GetFD(mb), MPI_MB_GetSize(mb),
               MPI_MB_GetModeID(mb), MPI_MB_GetChannelID(mb),
               MPI_MB_GetTimestamp(mb));

        if (fp_mp4)
            mp4_muxer_write(MPI_MB_GetPtr(mb), MPI_MB_GetSize(mb));

        MPI_MB_ReleaseBuffer(mb);
    }

    mp4_muxer_destroy();
    if (fp_mp4)
        fclose(fp_mp4);

    return NULL;
}

/// usage:
/// #./test-mp4 -d/dev/video0 -o/tmp/out.mp4
int main(int argc, char *argv[])
{
    int ret = -1;
    char *pDeviceName = "/dev/video0";
    int c = 0;
    opterr = 1;
    while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'w':
            g_width = atoi(optarg);
            break;
        case 'h':
            g_height = atoi(optarg);
            break;
        case 'c':
            g_frameCnt = atoi(optarg);
            break;
        case 'o':
            g_pOutPath = optarg;
             break;
        case 'd':
            pDeviceName = optarg;
            break;       
        case '?':
        default:
            print_usage(argv[0]);
            return 0;
        }
    }

    MPI_SYS_Init();

    HGVI_CONFIG_S stViAttr;
    stViAttr.dev = pDeviceName;
    stViAttr.chn = g_viChn;
    stViAttr.width = g_width;
    stViAttr.height =g_height;
    ret = MPI_VI_CreateChn(stViAttr);
    if (ret != ERR_SYS_OK)
    {
        printf("MPI_VI_CreateChn err.\n");
    }

    HGVENC_CONFIG_S stVencAttr;
    stVencAttr.chn = g_vencChn;
    stVencAttr.width = g_width;
    stVencAttr.height = g_height;
    stVencAttr.virWidth = g_width;
    stVencAttr.virHeight = g_height;
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
    if (ret != ERR_SYS_OK)
    {
        printf("MPI_VENC_CreateChn err.\n");
    }

    HGMPP_CHN_S stSourceChn;
    stSourceChn.modId = HGID_VI;
    stSourceChn.devId = 0;
    stSourceChn.chnId = g_viChn;
    HGMPP_CHN_S stSinkChn;
    stSinkChn.modId = HGID_VENC;
    stSinkChn.devId = 0;
    stSinkChn.chnId = g_vencChn;
    ret = MPI_SYS_Bind(&stSourceChn, &stSinkChn);
    if (ret != ERR_SYS_OK)
    {
        printf("Bind vi[%d] and venc[%d] failed! ret=%d\n", g_viChn, g_vencChn, ret);
        return -1;
    }

    printf("%s initial finish\n", __func__);
    signal(SIGINT, sigterm_handler);

    pthread_t thid;
    pthread_create(&thid, NULL, GetMediaBuffer, NULL);

    while (!g_quit)
    {
        usleep(100000);
    }

    // loop in here, wait thread
    pthread_join(thid, NULL);
    printf("%s exit!\n", __func__);
    
    MPI_SYS_UnBind(&stSourceChn, &stSinkChn);
    MPI_VENC_DestoryChn(g_vencChn);
    MPI_VI_DestoryChn(g_viChn);

    return 0;
}
