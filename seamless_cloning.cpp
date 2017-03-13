#include "seamless_cloning.h"
#include "cloning.h"

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/photo/photo.hpp>

#include "utils.h"

using namespace std;
using namespace cv;

void seamlessClone(cv::InputArray _src, cv::InputArray _dst,
                   cv::InputArray _mask, cv::Point p, cv::OutputArray _blend,
                   int flags) {
  Mat src = _src.getMat();
  Mat dest = _dst.getMat();
  Mat mask = _mask.getMat();
  _blend.create(dest.size(), CV_8UC3);
  Mat blend = _blend.getMat();

  int h = mask.rows;
  int w = mask.cols;
  int minx = w, miny = h, maxx = 0, maxy = 0;

  Mat gray = Mat(mask.size(), CV_8UC1);
  Mat dst_mask = Mat::zeros(dest.size(), CV_8UC1);
  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);
  Mat cd_mask = Mat::zeros(dest.size(), CV_8UC3);

  if (mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  // Mask bounding box
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (gray.at<uchar>(j, i) != 0) {
        minx = std::min(minx, i);
        maxx = std::max(maxx, i);
        miny = std::min(miny, j);
        maxy = std::max(maxy, j);
      }
    }
  }
  int lenx = maxx - minx;
  int leny = maxy - miny;

  int minxd = p.x;
  int maxxd = p.x + lenx;
  int minyd = p.y;
  int maxyd = p.y + leny;

  if (maxxd > dest.cols || maxyd > dest.rows) {
    cout << "Index out of range" << endl;
    return;
  }

  // Move point
  minxd = p.x + minx;
  minyd = p.y + miny;

  Rect roi_d(minxd, minyd, lenx, leny);
  Rect roi_s(minx, miny, lenx, leny);

  Mat destinationROI = dst_mask(roi_d);
  Mat sourceROI = cs_mask(roi_s);

  gray(roi_s).copyTo(destinationROI);
  src(roi_s).copyTo(sourceROI, gray(roi_s));

  destinationROI = cd_mask(roi_d);
  cs_mask(roi_s).copyTo(destinationROI);

  Cloning obj;
  obj.normal_clone(dest, cd_mask, dst_mask, blend, flags);
}

void colorChange(cv::InputArray _src, cv::InputArray _mask,
                 cv::OutputArray _dst, float r, float g, float b) {
  Mat src = _src.getMat();
  Mat mask = _mask.getMat();
  _dst.create(src.size(), src.type());
  Mat blend = _dst.getMat();

  float red = r;
  float green = g;
  float blue = b;

  Mat gray = Mat::zeros(mask.size(), CV_8UC1);

  if (mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);

  src.copyTo(cs_mask, gray);

  Cloning obj;
  obj.local_color_change(src, cs_mask, gray, blend, red, green, blue);
}

void illuminationChange(cv::InputArray _src, cv::InputArray _mask,
                        cv::OutputArray _dst, float a, float b) {
  Mat src = _src.getMat();
  Mat mask = _mask.getMat();
  _dst.create(src.size(), src.type());
  Mat blend = _dst.getMat();
  float alpha = a;
  float beta = b;

  Mat gray = Mat::zeros(mask.size(), CV_8UC1);

  if (mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);

  src.copyTo(cs_mask, gray);

  Cloning obj;
  obj.illum_change(src, cs_mask, gray, blend, alpha, beta);
}

void textureFlattening(cv::InputArray _src, cv::InputArray _mask,
                       cv::OutputArray _dst, double low_threshold,
                       double high_threshold, int kernel_size) {
  Mat src = _src.getMat();
  Mat mask = _mask.getMat();
  _dst.create(src.size(), src.type());
  Mat blend = _dst.getMat();

  Mat gray = Mat::zeros(mask.size(), CV_8UC1);

  if (mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  Mat cs_mask = Mat::zeros(src.size(), CV_8UC3);

  src.copyTo(cs_mask, gray);

  Cloning obj;
  obj.texture_flatten(src, cs_mask, gray, low_threshold, high_threshold,
                      kernel_size, blend);
}
