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

void contour2mask(const Path &path, Mat &mask)
{
  vector<vector<Point> > contours;
  contours.push_back(path);
  fillPoly(mask, contours, Scalar(255, 255, 255));
}

void mask2contour(const Mat &mask, Path &contour)
{
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  // Find contours
  findContours(mask, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

  if(contours.size() > 1)
  {
    cerr << "Something went wrong! Make sure the whole is a close.\n";
  }

  contour = contours[0];
}

void draw_path(const Mat &in, Path &path, Mat &out, const Scalar &color)
{
  Point prev_point, point;
  int thickness = 4;

  // Copy image
  in.copyTo(out);

  // first point
  prev_point = point = path[0];
  line(out, prev_point, point, color, thickness);

  // remaining point
  for(size_t i=1; i<path.size(); i++)
  {
    point = path[i];
    line(out, prev_point, point, color, thickness);
    prev_point = point;
  }
}

void close_path(const Path &path, Path &closed_path, unsigned extend_to)
{
  closed_path = path;

  // Closing contour
  Point first = path.front();
  if(extend_to>0)
  {
    Point last = path.back();
    Point new_last(last.x, extend_to);
    Point new_first(first.x, extend_to);
    closed_path.push_back(new_last);
    closed_path.push_back(new_first);
  }
  closed_path.push_back(first);
}
