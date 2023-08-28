#ifndef __HGMEDIA_H_
#define __HGMEDIA_H_

#include "hgmedia_types.h"

namespace hgdf {

int MPI_VI_CreateChn(HGVI_CONFIG_S& param);
int MPI_VI_StartStream(int chn);
int MPI_VI_DestoryChn(int chn);

int MPI_VPSS_CreateChn(HGVPSS_CONFIG_S& param);
int MPI_VPSS_DestoryChn(int chn);

int MPI_VENC_CreateChn(HGVENC_CONFIG_S& param);
int MPI_VENC_CreateMjpegChn(int chn, int width, int height);
int MPI_VENC_DestoryChn(int chn);
int MPI_VENC_StartRecvFrame(int chn, int recvPicNum);

int MPI_SYS_Init();
int MPI_SYS_Bind(const HGMPP_CHN_S* src, const HGMPP_CHN_S* dst);
int MPI_SYS_UnBind(const HGMPP_CHN_S* src, const HGMPP_CHN_S* dst);
int MPI_SYS_RegisterOutCb(const HGMPP_CHN_S* chn, HgOutCbFunc cb);
int MPI_SYS_SendMediaBuffer(HGMOD_ID_E enModId, int chnId, HGMEDIA_BUFFER buffer);
HGMEDIA_BUFFER MPI_SYS_GetMediaBuffer(HGMOD_ID_E enModId, int chnId, int ms);

int MPI_MB_ReleaseBuffer(HGMEDIA_BUFFER mb);
void* MPI_MB_GetPtr(HGMEDIA_BUFFER mb);
int MPI_MB_GetFD(HGMEDIA_BUFFER mb);
int MPI_MB_GetSize(HGMEDIA_BUFFER mb);
HGMOD_ID_E MPI_MB_GetModeID(HGMEDIA_BUFFER mb);
int MPI_MB_GetChannelID(HGMEDIA_BUFFER mb);
uint64_t MPI_MB_GetTimestamp(HGMEDIA_BUFFER mb);
HGMEDIA_BUFFER MPI_MB_CreateImageBuffer(HGIMAGE_INFO_S *pstImageInfo, HGBOOL hardware, uint8_t flag);
int MPI_MB_SetSize(HGMEDIA_BUFFER mb, uint32_t size);
int MPI_MB_SetTimestamp(HGMEDIA_BUFFER mb, uint64_t timestamp);

}

#endif // __HGMEDIA_H_
