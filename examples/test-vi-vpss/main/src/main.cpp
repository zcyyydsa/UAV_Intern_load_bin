#include "hgmedia.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

using namespace hgdf;

int8_t g_viChn = 0;
int8_t g_vpssChn = 0;

static bool quit = false;
static void sigterm_handler(int sig)
{
    fprintf(stderr, "signal %d\n", sig);
    quit = true;
}

static void *GetMediaBuffer(void *arg)
{
    printf("#Start %s thread, arg:%p\n", __func__, arg);
    const char *save_path = "/tmp/out_nv12.yuv";
    FILE *save_file = fopen(save_path, "w");
    if (!save_file)
        printf("ERROR: Open %s failed!\n", save_path);

    HGMEDIA_BUFFER mb = NULL;
    int save_cnt = 0;
    while (!quit)
    {
        mb = MPI_SYS_GetMediaBuffer(HGID_VPSS, g_vpssChn, -1);
        if (!mb)
        {
            printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
            break;
        }

        printf("Get Frame:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
               "timestamp:%lld\n",
               MPI_MB_GetPtr(mb), MPI_MB_GetFD(mb), MPI_MB_GetSize(mb),
               MPI_MB_GetModeID(mb), MPI_MB_GetChannelID(mb),
               MPI_MB_GetTimestamp(mb));

        if (save_file && (save_cnt < 3))
        {
            fwrite(MPI_MB_GetPtr(mb), 1, MPI_MB_GetSize(mb), save_file);
            printf("#Save frame-%d to %s\n", save_cnt, save_path);
            save_cnt++;
        }

        MPI_MB_ReleaseBuffer(mb);
    }

    if (save_file)
        fclose(save_file);

    return NULL;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *pDeviceName = "/dev/video1";
    uint32_t width = 1280;
    uint32_t height = 720;
    uint32_t scale_width = 640;
    uint32_t scale_height = 480;

    MPI_SYS_Init();

    HGVI_CONFIG_S stViAttr;
    stViAttr.dev = pDeviceName;
    stViAttr.chn = g_viChn;
    stViAttr.width = width;
    stViAttr.height = height;
    ret = MPI_VI_CreateChn(stViAttr);
    if (ret != ERR_SYS_OK)
    {
        printf("Create vi[%d] failed! ret=%d\n", g_viChn, ret);
        return -1;
    }

    HGVPSS_CONFIG_S stVpssAttr;
    stVpssAttr.chn = g_vpssChn;
    stVpssAttr.srcType = HGIMAGE_TYPE_NV12;
    stVpssAttr.srcWidth = HGUPALIGNTO16(width);
    stVpssAttr.srcHeight = HGUPALIGNTO16(height);
    stVpssAttr.srcHorStride = HGUPALIGNTO16(width);
    stVpssAttr.srcVirStride = HGUPALIGNTO16(height);
    stVpssAttr.dstType = HGIMAGE_TYPE_NV12;
    stVpssAttr.dstWidth = HGUPALIGNTO16(scale_width);
    stVpssAttr.dstHeight = HGUPALIGNTO16(scale_height);
    stVpssAttr.dstHorStride = HGUPALIGNTO16(scale_width);
    stVpssAttr.dstVirStride = HGUPALIGNTO16(scale_height);
    ret = MPI_VPSS_CreateChn(stVpssAttr);
    if (ret != ERR_SYS_OK)
    {
        printf("Create vpss[%d] failed! ret=%d\n", g_vpssChn, ret);
        return -1;
    }

    HGMPP_CHN_S stSourceChn;
    stSourceChn.modId = HGID_VI;
    stSourceChn.devId = 0;
    stSourceChn.chnId = g_viChn;
    HGMPP_CHN_S stSinkChn;
    stSinkChn.modId = HGID_VPSS;
    stSinkChn.devId = 0;
    stSinkChn.chnId = g_vpssChn;
    ret = MPI_SYS_Bind(&stSourceChn, &stSinkChn);
    if (ret != ERR_SYS_OK)
    {
        printf("Bind vi[%d] and vpss[%d] failed! ret=%d\n", g_viChn, g_vpssChn, ret);
        return -1;
    }

    pthread_t thid;
    pthread_create(&thid, NULL, GetMediaBuffer, NULL);

    printf("%s initial finish\n", __func__);
    signal(SIGINT, sigterm_handler);

    while (!quit) {
        usleep(100);
    }

    // loop in here, wait thread
    pthread_join(thid, NULL);

    MPI_SYS_UnBind(&stSourceChn, &stSinkChn);
    MPI_VPSS_DestoryChn(g_vpssChn);
    MPI_VI_DestoryChn(g_viChn);
    return 0;
}
