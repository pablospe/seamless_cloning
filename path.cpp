#include "path.h"

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

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
