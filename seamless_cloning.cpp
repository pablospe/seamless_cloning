#include "seamless_cloning.h"
#include "cloning.h"

#include <iostream>
#include <opencv2/photo/photo.hpp>

using namespace std;
using namespace cv;

void seamlessClone(cv::InputArray _src,
                   cv::InputArray _dst,
                   cv::InputArray _mask,
                   cv::Point p,
                   cv::OutputArray _blend,
                   int flags)
{
  Mat src  = _src.getMat();
  Mat dest = _dst.getMat();
  Mat mask = _mask.getMat();
  _blend.create(dest.size(), CV_8UC3);
  Mat blend = _blend.getMat();

  int minx = INT_MAX, miny = INT_MAX, maxx = INT_MIN, maxy = INT_MIN;
  int h = mask.size().height;
  int w = mask.size().width;

  Mat gray = Mat(mask.size(), CV_8UC1);
  Mat dst_mask = Mat::zeros(dest.size(), CV_8UC1);
  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);
  Mat cd_mask = Mat::zeros(dest.size(), CV_8UC3);

  if(mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  for(int i = 0; i < h; i++)
  {
    for(int j = 0; j < w; j++)
    {
      if(gray.at<uchar> (i, j) == 255)
      {
        minx = std::min(minx, i);
        maxx = std::max(maxx, i);
        miny = std::min(miny, j);
        maxy = std::max(maxy, j);
      }
    }
  }

  int lenx = maxx - minx;
  int leny = maxy - miny;

  int minxd = p.y - lenx / 2;
  int maxxd = p.y + lenx / 2;
  int minyd = p.x - leny / 2;
  int maxyd = p.x + leny / 2;

  if(minxd < 0 || minyd < 0 || maxxd > dest.size().height || maxyd > dest.size().width)
  {
    cout << "Index out of range" << endl;
    exit(0);
  }

  Rect roi_d(minyd, minxd, leny, lenx);
  Rect roi_s(miny, minx, leny, lenx);

  Mat destinationROI = dst_mask(roi_d);
  Mat sourceROI = cs_mask(roi_s);

  gray(roi_s).copyTo(destinationROI);
  src(roi_s).copyTo(sourceROI, gray(roi_s));

  destinationROI = cd_mask(roi_d);
  cs_mask(roi_s).copyTo(destinationROI);

  Cloning obj;
  obj.normal_clone(dest, cd_mask, dst_mask, blend, flags);
}

void colorChange(cv::InputArray _src,
                 cv::InputArray _mask,
                 cv::OutputArray _dst,
                 float r,
                 float g,
                 float b)
{
  Mat src  = _src.getMat();
  Mat mask  = _mask.getMat();
  _dst.create(src.size(), src.type());
  Mat blend = _dst.getMat();

  float red = r;
  float green = g;
  float blue = b;

  Mat gray = Mat::zeros(mask.size(), CV_8UC1);

  if(mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);

  src.copyTo(cs_mask, gray);

  Cloning obj;
  obj.local_color_change(src, cs_mask, gray, blend, red, green, blue);
}


void illuminationChange(cv::InputArray _src,
                        cv::InputArray _mask,
                        cv::OutputArray _dst,
                        float a,
                        float b)
{

  Mat src  = _src.getMat();
  Mat mask  = _mask.getMat();
  _dst.create(src.size(), src.type());
  Mat blend = _dst.getMat();
  float alpha = a;
  float beta = b;

  Mat gray = Mat::zeros(mask.size(), CV_8UC1);

  if(mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);

  src.copyTo(cs_mask, gray);

  Cloning obj;
  obj.illum_change(src, cs_mask, gray, blend, alpha, beta);

}

void textureFlattening(cv::InputArray _src,
                       cv::InputArray _mask,
                       cv::OutputArray _dst,
                       double low_threshold,
                       double high_threshold,
                       int kernel_size)
{

  Mat src  = _src.getMat();
  Mat mask  = _mask.getMat();
  _dst.create(src.size(), src.type());
  Mat blend = _dst.getMat();

  Mat gray = Mat::zeros(mask.size(), CV_8UC1);

  if(mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);

  src.copyTo(cs_mask, gray);

  Cloning obj;
  obj.texture_flatten(src, cs_mask, gray, low_threshold, high_threshold, kernel_size, blend);
}
