#ifndef __HG_TRACK_H_
#define __HG_TRACK_H_

#include <stdint.h>

class Track;
class HgTrack
{
public:
    HgTrack();
    ~HgTrack();

    int extinit(int x, int y, int w, int h, uint8_t* img, int imgW, int imgH);
    float extupdate(uint8_t* img, int imgW, int imgH, int& x, int&y, int &w, int &h);

private:
    Track* track_;
};

#endif
