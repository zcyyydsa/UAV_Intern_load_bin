
#include "hgmedia.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

using namespace hgdf;

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
    char *ot_path = (char *)arg;
    printf("#Start %s thread, arg:%p, out path: %s\n", __func__, arg, ot_path);
    FILE *save_file = fopen(ot_path, "w");
    if (!save_file)
        printf("ERROR: Open %s failed!\n", ot_path);

    HGMEDIA_BUFFER mb = NULL;
    while (!g_quit)
    {
        mb = MPI_SYS_GetMediaBuffer(HGID_VENC, 0, -1);
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

        if (save_file)
            fwrite(MPI_MB_GetPtr(mb), 1, MPI_MB_GetSize(mb), save_file);
        MPI_MB_ReleaseBuffer(mb);
    }

    if (save_file)
        fclose(save_file);

    return NULL;
}

/// usage:
/// #./test-venc -i/tmp/out_nv12.yuv -o/tmp/out.h264
int main(int argc, char *argv[])
{
    int ret = -1;
    char *input_file = "/tmp/1080p_nv12.yuv";
    char *output_file = "/tmp/out.h264";
    uint32_t width = 1920;
    uint32_t height = 1080;
    uint8_t vencChn = 0;
    int c = 0;
    opterr = 1;
    while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'i':
            input_file = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case '?':
        default:
            print_usage(argv[0]);
            return 0;
        }
    }

    MPI_SYS_Init();

    HGVENC_CONFIG_S stVencAttr;
    stVencAttr.chn = vencChn;
    stVencAttr.width = width;
    stVencAttr.height = height;
    stVencAttr.virWidth = width;
    stVencAttr.virHeight = height;
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
    if (ret != ERR_SYS_OK)
    {
        printf("MPI_VENC_CreateChn err.\n");
    }

    printf("%s initial finish\n", __func__);
    signal(SIGINT, sigterm_handler);

    FILE *read_file = fopen(input_file, "r");
    if (!read_file)
    {
        printf("ERROR: open %s failed!\n", input_file);
        exit(0);
    }

    HGIMAGE_INFO_S stImageInfo = {1920, 1080, 1920, 1080, HGIMAGE_TYPE_NV12};

    pthread_t read_thread;
    pthread_create(&read_thread, NULL, GetMediaBuffer, output_file);

    uint32_t u32FrameId = 0;
    uint32_t s32ReadSize = 0;
    uint64_t u64TimePeriod = 33333; // us
    while (!g_quit)
    {
        // Create dma buffer. Note that mpp encoder only support dma buffer.
        HGMEDIA_BUFFER mb =
            MPI_MB_CreateImageBuffer(&stImageInfo, TURE, 1 /*MB_FLAG_NOCACHED*/);
        if (!mb)
        {
            printf("ERROR: no space left!\n");
            break;
        }

        // One frame size for nv12 image.
        // 3110400 = 1920 * 1080 * 3 / 2;
        s32ReadSize = fread(MPI_MB_GetPtr(mb), 1, 3110400, read_file);
        if (s32ReadSize != 3110400)
        {
            printf("Get end of file!\n");
            break;
        }
        MPI_MB_SetSize(mb, 3110400);
        MPI_MB_SetTimestamp(mb, u32FrameId * u64TimePeriod);
        printf("#Send frame[%d] fd=%d to venc[0]...\n", u32FrameId++, MPI_MB_GetFD(mb));
        MPI_SYS_SendMediaBuffer(HGID_VENC, vencChn, mb);
        // mb must be release. The encoder has internal references to the data sent
        // in. Therefore, mb cannot be reused directly
        MPI_MB_ReleaseBuffer(mb);
        usleep(u64TimePeriod);
    }

    while (!g_quit)
    {
        usleep(100000);
    }

    printf("%s exit!\n", __func__);
    MPI_VENC_DestoryChn(vencChn);

    return 0;
}
