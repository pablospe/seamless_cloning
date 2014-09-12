#include "utils.h"

#include <iostream>
#include <iomanip>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

void display_blending(const string &winname,
                      InputArray _src1,
                      InputArray _src2,
                      int steps,
                      bool animation,
                      int delayms)
{
  // getMat
  Mat src1 = _src1.getMat();
  Mat src2 = _src2.getMat();

  // Show 'src1' (no initial linear blend)
  float alpha = 1.f, beta = 0.f;
  Mat blend;
  addWeighted(src1, alpha, src2, beta, 0.0, blend);
  imshow(winname, blend);

  // Internal variables
  int count = 0, last_count = 0, sgn = -1;

  // Show Trackbar
  if (!animation)
    createTrackbar("Alpha", winname, &count, steps);

  // Press 'q' or ESC (27) to exit
  char k;
  while ((k = waitKey(delayms)) != 'q' && k != 27)
  {
    if (animation)
    {
      // Stop animation pressing 's' or Space
      if(k == 's' || k == ' ')
      {
        animation = false;
        createTrackbar("Alpha", winname, &count, steps);
        displayStatusBar(winname, "");
        continue;
      }

      // Change direction
      if (count <= 0 || count >= steps)
        sgn = -sgn;

      // Increase/decrease depending of 'sgn'
      string text;
      if (sgn > 0)
      {
        count++;
        text = " -> ";
      }
      else
      {
        count--;
        text = " <- ";
      }

      // Show status
      stringstream status;
      status << '(' << count << '/' << steps << ')';
      displayStatusBar(winname, text + status.str());
    }

    // Update linear blend
    if (last_count != count)
    {
      beta = static_cast<float>(count) / steps;
      alpha = 1.f - beta;
      Mat blend;
      addWeighted(src1, alpha, src2, beta, 0.0, blend);
      imshow(winname, blend);
      last_count = count;
    }
  }
}

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
