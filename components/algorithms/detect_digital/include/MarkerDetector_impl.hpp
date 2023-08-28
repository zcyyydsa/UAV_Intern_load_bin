#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <cstddef> //c++头文件，包含一些重定义的类型，size_t
#include <opencv2/opencv.hpp>

#include "Marker.hpp"

//FL is findLineFromContour
#define FL_VALID_SIZE   10 
#define FL_ANGLE0       (45.0/57.3)
#define FL_NUM0         5

using namespace cv;
using namespace std;

//定义一个标记检测类：MarkerDetector
class MarkerDetector
{
public:
  typedef vector<Point> PointsVector;
  typedef vector<PointsVector> ContoursVector;

  MarkerDetector(string template_file_name);

  void processFrame(const Mat& frame,vector<Marker>& markers);

  const vector<Transformation>& getTransformations() const;

public:
  void prepareImage(const Mat& src,Mat& grayscale);
  void performThreshold(const Mat& grayscale,Mat& thresholdImg);
  void findContour(cv::Mat& thresholdImg, ContoursVector& contours, int minContourPointsAllowed,Mat im) const;
  void findCandidates(const ContoursVector& contours,vector<Marker>& detectedMarkers);
  void findCandidates0(const ContoursVector& contours0,vector<Marker>& detectedMarkers);

  void recognizeMarkers(const Mat& grayscale,vector<Marker>& detectedMarkers);
  void recognizeMarkers0(const Mat& grayscale,vector<Marker>& detectedMarkers);
  void estimatePosition(vector<Marker>& detectedMarkers,Mat_<float>& camMatrix,Mat_<float>& distCoeff);
  bool findMarkers(const Mat& frame,vector<Marker>& detectedMarkers);
  void findLineFromContour(ContoursVector& contours,ContoursVector& contours_out) const;
  void calNearestPoint(Point2f& target_p,PointsVector contour,int len);
public:

  Size markerSize;

  ContoursVector m_contours;
  vector<Point3f> m_markerCorners3d;
  vector<Point2f> m_markerCorners2d;

  Mat m_grayscaleImage;
  Mat m_thresholdImg;
  Mat canonicalMarkerImage;

  vector<Transformation> m_transformations;

  float m_minContourLengthAllowed;

  //0 failed to open file imcode.yml
  int mStatus;
  vector<string> mvTemplateNames;
  vector<Mat> mvTemplateData;
  // vector<Mat> mvTemplateData90;
  // vector<Mat> mvTemplateData180;
  // vector<Mat> mvTemplateData270;
};

struct BGRAVideoFrame
{
  size_t width;
  size_t height;
  size_t stride;

  unsigned char * data;
};

template <typename T>
string ToString(const T& value)
{
    ostringstream stream;
    stream << value;
    return stream.str();
}



