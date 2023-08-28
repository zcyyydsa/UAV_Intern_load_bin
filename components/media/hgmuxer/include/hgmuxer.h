#ifndef __HGMUXER_H_
#define __HGMUXER_H_

namespace hgdf {

void mp4_muxer_create(void* param, int width, int height);
int mp4_muxer_write(const void* data, int size);
void mp4_muxer_destroy();

}

#endif // __HGMUXER_H_