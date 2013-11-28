#include "quadrilateral.h"

#include <iostream>
#include <iomanip>

using namespace std;
using namespace cv;

Quadrilateral::Quadrilateral() : P_(4)
{
}

Quadrilateral::Quadrilateral(const Quadrilateral &other)
{
  operator=(other);
}

Quadrilateral::Quadrilateral(const Rect_<float> &R)
{
  P_.resize(4);
  fromRect(R);
}

Quadrilateral::Quadrilateral(const Size &S)
{
  P_.resize(4);
  fromSize(S);
}

Quadrilateral::~Quadrilateral()
{
}

const Quadrilateral &Quadrilateral::operator=(const Quadrilateral &other)
{
  P_[0] = other.P_[0];
  P_[1] = other.P_[1];
  P_[2] = other.P_[2];
  P_[3] = other.P_[3];

  return other;
}

void Quadrilateral::fromRect(const cv::Rect_<float> &R)
{
  P_[0] = Point2f(R.x, R.y);
  P_[1] = Point2f(R.x + R.width, R.y);
  P_[2] = Point2f(R.x + R.width, R.y + R.height);
  P_[3] = Point2f(R.x, R.y + R.height);
}

void Quadrilateral::fromSize(const cv::Size &S)
{
  int h = S.height;
  int w = S.width;

  P_[0] = Point2f(0, 0);
  P_[1] = Point2f(w, 0);
  P_[2] = Point2f(w, h);
  P_[3] = Point2f(0, h);
}

void Quadrilateral::print()
{
  cout << std::fixed << setprecision(3)
       << "P[0] = " << P_[0] << " (left-top)\n"
       << "P[1] = " << P_[1] << " (right-top)\n"
       << "P[2] = " << P_[2] << " (right-bottom)\n"
       << "P[3] = " << P_[3] << " (left-bottom)\n";
}

Rect Quadrilateral::inner_rect()
{
  return Rect(Point2f(left_max(), top_max()),
              Point2f(right_min(), bottom_min()));
}

Rect Quadrilateral::outer_rect()
{
  return Rect(Point2f(left_min(), top_min()),
              Point2f(right_max(), bottom_max()));
}

void Quadrilateral::apply_homography(const Mat &H)
{
  vector<Point2f> projected_points(4);
  perspectiveTransform(P_, projected_points, H);
  P_ = projected_points;
}
