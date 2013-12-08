#include "utils.h"

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

void cut_and_paste(cv::Mat &dst,
                   const cv::Mat &src,
                   const cv::Point &offset)
{
  Mat mask(src.size(), CV_8UC1, 255);
  cut_and_paste(dst, src, mask, offset);
}

void cut_and_paste(cv::Mat &dst,
                   const cv::Mat &src,
                   const cv::Mat &mask,
                   const cv::Point &offset)
{
  if(offset.x < 0 || offset.y < 0   ||
     offset.x + src.cols > dst.cols ||
     offset.y + src.rows > dst.rows)
  {
    cout << "Out of range" << endl;
    return;
  }

  Mat dst_roi = dst(Rect(offset.x, offset.y, src.cols, src.rows));
  src.copyTo(dst_roi, mask);
}

void get_bounding_box(const Mat &mask, Rect &roi)
{
  Mat gray = Mat(mask.size(), CV_8UC1);

  if(mask.channels() == 3)
    cvtColor(mask, gray, COLOR_BGR2GRAY);
  else
    gray = mask;

  int h = mask.rows;
  int w = mask.cols;
  int minx = w, miny = h, maxx = 0, maxy = 0;

  // Mask bounding box
  for(int j = 0; j < h; j++)
  {
    for(int i = 0; i < w; i++)
    {
      if(gray.at<uchar>(j, i) == 255)
      {
        minx = min(minx, i);
        maxx = max(maxx, i);
        miny = min(miny, j);
        maxy = max(maxy, j);
      }
    }
  }
  int lenx = maxx - minx;
  int leny = maxy - miny;

  roi = Rect(minx, miny, lenx, leny);
}

void draw_rect(cv::Mat &img,
               const cv::Rect &roi,
               const cv::Scalar &color)
{
  rectangle(img, Point(roi.x,roi.y), Point(roi.x+roi.width, roi.y+roi.height), color, 1, 8, 0);
}
