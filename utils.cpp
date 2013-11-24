#include "utils.h"

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

void cutpaste(const Point &p, const Mat &src, Mat &dst)
{
  if(p.x < 0 || p.y < 0 || p.x + src.cols > dst.cols || p.y + src.rows > dst.rows)
  {
    cout << "Out of range" << endl;
    return;
  }

  Mat dst_roi = dst(Rect(p.x, p.y, src.cols, src.rows));
  src.copyTo(dst_roi);
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
