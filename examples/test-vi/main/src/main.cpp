#include "hgmedia.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <pthread.h>

using namespace hgdf;

uint32_t g_frameCnt = 10;
int8_t g_viChn = 0;
char *g_pOutPath = "/tmp/out_nv12.yuv";

static char optstr[] = "?:a::w:h:c:o:e:";
static const struct option long_options[] = {
    {"device_name", required_argument, NULL, 'd'},
    {"width", required_argument, NULL, 'w'},
    {"height", required_argument, NULL, 'h'},
    {"frame_cnt", required_argument, NULL, 'c'},
    {"output_path", required_argument, NULL, 'o'},
    {"encode", required_argument, NULL, 'e'},
    {NULL, 0, NULL, 0},
};

static void print_usage(const char *name)
{
    printf("usage example:\n");

    printf("\t%s [-w | --width 1920] "
           "[-h | --heght 1080]"
           "[-c | --frame_cnt 150] "
           "[-d | --device_name /dev/video0] "
           "[-o | --output_path /tmp/out_nv12.yuv] \n",
           name);

    printf("\t-w | --width: VI width, Default:1920\n");
    printf("\t-h | --heght: VI height, Default:1080\n");
    printf("\t-c | --frame_cnt: frame number of output, Default:10\n");
    printf("\t-d | --device_name set pcDeviceName, Default:/dev/video0, "
           "Option:[/dev/video0, /dev/video1]\n");
    printf("\t-o | --output_path: output path, Default:/tmp/out_nv12.yuv\n");
    printf("Notice: fmt always NV12\n");
}


static void* vi_thread(void *arg) {
    printf("#Start %s thread, arg:%p\n", __func__, arg);

    HGMEDIA_BUFFER mb = NULL;

    //int w_virtual = UPALIGNTO(in_w, 8);
    //int h_virtual = UPALIGNTO(in_h, 8);

    //printf("\n#muxer floe param:\n%s\n", param.c_str());
    int cnt = 0;

    FILE *save_file = fopen(g_pOutPath, "w");
    if (!save_file)
        printf("open %s failed!\n", g_pOutPath);

    while (cnt != g_frameCnt) {
        mb = MPI_SYS_GetMediaBuffer(HGID_VI, g_viChn, -1);
        if (!mb) {
            printf("MPI_SYS_GetMediaBuffer get null buffer!\n");
            break;
        }

        printf("Get vi packet[%d]:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
            "timestamp:%lld\n",
            cnt+1, MPI_MB_GetPtr(mb), /*MPI_MB_GetFD(mb)*/cnt,
            MPI_MB_GetSize(mb), MPI_MB_GetModeID(mb),
            MPI_MB_GetChannelID(mb), MPI_MB_GetTimestamp(mb));

        if (save_file)
            fwrite(MPI_MB_GetPtr(mb), 1, MPI_MB_GetSize(mb), save_file);     

        cnt ++;
        MPI_MB_ReleaseBuffer(mb);
    }

    if (save_file)
        fclose(save_file);

    return NULL;
}

int main(int argc, char *argv[])
{
    uint32_t width = 1920;
    uint32_t height = 1080;
    char *pDeviceName = "/dev/video0";
    

    int c;
    int ret = 0;
    while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1)
    {
        printf("get op is %d, a is %d\n", c, 'a');
        const char *tmp_optarg = optarg;
        switch (c)
        {
        case 'w':
            width = atoi(optarg);
            break;
        case 'h':
            height = atoi(optarg);
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

    printf(">>>>>>>>>>>>>>> Test START <<<<<<<<<<<<<<<<<<<<<<\n");
    printf("#Device: %s\n", pDeviceName);
    printf("#Resolution: %dx%d\n", width, height);
    printf("#Frame Count to save: %d\n", g_frameCnt);
    printf("#Output Path: %s\n", g_pOutPath);

    MPI_SYS_Init();

    HGVI_CONFIG_S stViAttr;
    stViAttr.dev = pDeviceName;
    stViAttr.chn = g_viChn;
    stViAttr.width = width;
    stViAttr.height = height;
    ret = MPI_VI_CreateChn(stViAttr);
    if (ret != ERR_SYS_OK)
    {
        printf("MPI_VI_CreateChn err.\n");
    }
    MPI_VI_StartStream(g_viChn);
    
    pthread_t thid;
    pthread_create(&thid, NULL, vi_thread, NULL);

    // loop in here, until keyboard click
    // getchar();

    // loop in here, wait thread
    pthread_join(thid, NULL);

    MPI_VI_DestoryChn(g_viChn);
    return 0;
}
