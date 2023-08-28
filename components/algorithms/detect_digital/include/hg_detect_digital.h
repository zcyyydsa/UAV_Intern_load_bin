#ifndef HG_DETECT_DIGITAL_H
#define HG_DETECT_DIGITAL_H

#include <stdint.h>
#include "MarkerDetector_impl.hpp"

using namespace std;
using namespace cv;

class MarkerDetector;
class HgDDigtal{
public:
    HgDDigtal(string template_file_name="");
    ~HgDDigtal();
    void DetectDigital(Mat im,vector<Marker>& markers);

private:
    MarkerDetector* mpMD;
    vector<Marker> mMarkers;//result
};

#endif