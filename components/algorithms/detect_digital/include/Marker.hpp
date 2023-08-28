#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "GeometryTypes.hpp"

using namespace cv;
using namespace std;

class Marker
{
public:
  Marker();

  friend bool operator<(const Marker& M1,const Marker& M2);
  friend ostream & operator << (ostream &str,const Marker &M);

  static Mat rotate(Mat in);
  static int hammDistMarker(Mat bits);
  static int mat2id(const cv::Mat &bits);
  static int getMarkerId(Mat &in,int &nRotations);
  static int getMarkerId0(Mat &in,int &nRotations,vector<string>& templateNames,vector<Mat>& templateData);

public:
  //Id of the marker
  int id;

  //Marker transformation with regards to the camera
  Transformation transformation;

  vector<Point2f> points;

  //Helper function to draw the marker contour over the image
  void drawContour(Mat& image,Scalar color = CV_RGB(0,250,0)) const;
};
