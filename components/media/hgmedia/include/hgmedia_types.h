#ifndef __HGMEDIA_TYPES_H_
#define __HGMEDIA_TYPES_H_

#include "stdint.h"

typedef void *HGMEDIA_BUFFER;
typedef void (*HgOutCbFunc)(HGMEDIA_BUFFER mb);

#define HGUPALIGNTO(value, align) ((value + align - 1) & (~(align - 1)))
#define HGUPALIGNTO16(value) HGUPALIGNTO(value, 16)
#define HGDOWNALIGNTO16(value) (HGUPALIGNTO(value, 16) - 16)

typedef enum {
    FALSE = 0,
    TURE = 1,
} HGBOOL;

typedef enum _MOD_ID_E {
    HGID_UNKNOW = 0,  
    HGID_VENC = 4,
    HGID_VI = 9,
    HGID_VPSS = 17,

    HGID_BUTT,
} HGMOD_ID_E;

typedef enum _IMAGE_TYPE_E {
    HGIMAGE_TYPE_UNKNOW = 0,
    HGIMAGE_TYPE_NV12 = 4,
    HGIMAGE_TYPE_RGB888 = 18,
    HGIMAGE_TYPE_BGR888 = 19,

    HGIMAGE_TYPE_BUTT    
} HGIMAGE_TYPE_E;

typedef struct _IMAGE_INFO_S {
    uint32_t width;
    uint32_t height;
    uint32_t horStride;
    uint32_t verStride;
    HGIMAGE_TYPE_E enImgType;
} HGIMAGE_INFO_S;

typedef struct _dfMPP_CHN_S {
    HGMOD_ID_E modId;
    int devId;
    int chnId;
} HGMPP_CHN_S;

typedef struct _VI_CONFIG_S
{
    char* dev;
    int chn;
    int width;
    int height;
} HGVI_CONFIG_S;

typedef struct _VPSS_CONFIG_S
{
    int chn;

    HGIMAGE_TYPE_E srcType;
    uint32_t srcX;
    uint32_t srcY;
    int srcWidth;
    int srcHeight;
    int srcHorStride;
    int srcVirStride;
    
    HGIMAGE_TYPE_E dstType;
    uint32_t dstX;
    uint32_t dstY;
    int dstWidth;
    int dstHeight;
    int dstHorStride;
    int dstVirStride;

} HGVPSS_CONFIG_S;

typedef struct _VENC_CONFIG_S
{
    int chn;
    int width;
    int height;
    int virWidth;
    int virHeight;

    int profile;
    int gop;
    int bitRate;
    int qpSet;
    int startQp;
    int stepQp;
    int maxIQp;
    int minIQp;
    int maxQp;
    int minQp;
} HGVENC_CONFIG_S;

enum {
    /***********************************
     * Common error types
     **********************************/
    ERR_SYS_OK = 0,
    ERR_SYS_NULL_PTR,
    ERR_SYS_NOTREADY,
    ERR_SYS_NOT_PERM,
    ERR_SYS_NOMEM,
    ERR_SYS_ILLEGAL_PARAM,
    ERR_SYS_BUSY,
    ERR_SYS_NOT_SUPPORT,
    ERR_SYS_OP_REPEAT,
    ERR_SYS_ERR_STATUS,

    /***********************************
     * VideoInput error types
     **********************************/
    /* invlalid channel ID */
    ERR_VI_INVALID_CHNID = 10,
    /* system is busy*/
    ERR_VI_BUSY,
    /* channel exists */
    ERR_VI_EXIST,
    ERR_VI_NOT_CONFIG,
    ERR_VI_TIMEOUT,
    ERR_VI_BUF_EMPTY,
    ERR_VI_ILLEGAL_PARAM,
    ERR_VI_NOTREADY,

    /***********************************
     * VideoEncoder error types
     **********************************/
    /* invlalid channel ID */
    ERR_VENC_INVALID_CHNID = 20,
    /* at lease one parameter is illagal ,eg, an illegal enumeration value  */
    ERR_VENC_ILLEGAL_PARAM,
    /* channel exists */
    ERR_VENC_EXIST,
    /* channel exists */
    ERR_VENC_UNEXIST,
    /* using a NULL point */
    ERR_VENC_NULL_PTR,
    /* try to enable or initialize system,device or channel,
        before configing attrib */
    ERR_VENC_NOT_CONFIG,
    /* operation is not supported by NOW */
    ERR_VENC_NOT_SUPPORT,
    /* operation is not permitted ,eg, try to change stati attribute */
    ERR_VENC_NOT_PERM,
    /* failure caused by malloc memory */
    ERR_VENC_NOMEM,
    /* failure caused by malloc buffer */
    ERR_VENC_NOBUF,
    /* no data in buffer */
    ERR_VENC_BUF_EMPTY,
    /* no buffer for new data */
    ERR_VENC_BUF_FULL,
    /* system is not ready,had not initialed or loaded*/
    ERR_VENC_NOTREADY,
    /* system is busy*/
    ERR_VENC_BUSY,

    /***********************************
     * VPSS::vpss error types
     **********************************/
    /* invlalid channel ID */
    ERR_RGA_INVALID_CHNID = 90,
    /* system is busy*/
    ERR_RGA_BUSY,
    /* channel exists */
    ERR_RGA_EXIST,
    ERR_RGA_NOT_CONFIG,
    /* at lease one parameter is illagal ,eg, an illegal enumeration value  */
    ERR_RGA_ILLEGAL_PARAM,
    ERR_RGA_NOTREADY,

    ERR_BUIT,
};

#endif //__HGMEDIA_TYPES_H_