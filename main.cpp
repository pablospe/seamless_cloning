#include <iostream>
#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>

#include "seamless_cloning.h"
#include "utils.h"

using namespace std;
using namespace cv;

Mat src, dst, subimg, result;

Point point;
int drag = 0;
int destx = -1, desty = -1;

void cutpaste(const Point &p, const Mat &src, Mat &target)
{
  if(p.x<0 || p.y<0 || p.x+src.cols>target.cols || p.y+src.rows>target.rows)
  {
    cout << "Out of range" << endl;
    return;
  }

  Mat dst_roi = target(Rect(p.x, p.y, src.cols, src.rows));
  src.copyTo(dst_roi);
}

void selectPointHandler(int event, int x, int y, int flags, void *param)
{
  point = Point(x, y);
  cout << x << " " << y << endl;
//   cout.flush();
}

void mouseHandler(int event, int x, int y, int flags, void *param)
{
  if(event == CV_EVENT_LBUTTONDOWN && !drag)
  {
    cout << "CV_EVENT_LBUTTONDOWN && !drag\n";
    point = Point(x, y);
    drag  = 1;
  }
  else if(event == CV_EVENT_LBUTTONDOWN && drag)
  {
    cout << "CV_EVENT_LBUTTONDOWN && drag\n";
    if(x - point.x < 0 || y - point.y < 0)
      return;

    Rect roi(point.x, point.y, x - point.x, y - point.y);
    src(roi).copyTo(subimg);
    imshow("ROI", subimg);
    imshow("Source", src);
    waitKey(3);
    drag = 0;
  }

  if(event == CV_EVENT_MOUSEMOVE && drag)
  {
    Mat tmp;
    src.copyTo(tmp);
    rectangle(tmp, point, Point(x, y), CV_RGB(255, 0, 0), 1, 8, 0);
    imshow("Source", tmp);
  }

  if(event == CV_EVENT_RBUTTONUP)
  {
    imshow("Source", src);
    drag = 0;
  }
}


void mouseHandler1(int event, int x, int y, int flags, void *param)
{
  Mat im;
  dst.copyTo(im);

  if(event == CV_EVENT_LBUTTONDOWN)
  {
    rectangle(im, Point(x,y), Point(x + subimg.cols, y + subimg.rows),
              CV_RGB(255, 0, 0), 1, 8, 0);

    destx = x;
    desty = y;

    cout << x << " " << y << " "
         << subimg.cols << " " << subimg.rows << " "
         << dst.cols << " " << dst.rows << endl;

    imshow("Destination", im);
  }

  if(event == CV_EVENT_RBUTTONUP)
  {

    if(destx < 0 || desty < 0)
    {
      cout << "Select mask location (Left Click)" << endl;
      return;
    }

    if(destx + subimg.cols > dst.cols || desty + subimg.rows > dst.rows)
    {
      cout << "Index out of range" << endl;
      return;
    }

    // create mask same size as source, and in white (255) the selected subimg
    Mat mask  = Mat::zeros(src.size(), CV_8UC1);
    Mat white = Mat::ones(subimg.size(), CV_8UC1)*255;
    cutpaste(point, white, mask);

    Rect r(point.x, point.y, white.cols, white.rows);
    cout << "Bounding box: " << r << endl;



    seamlessClone(src, dst, mask, Point(destx, desty), result, 1);

    ////////// save blended result ////////////////////
    imwrite("Output.jpg", result);

    cutpaste(Point(destx, desty), subimg, im);
    imwrite("cutpaste.jpg", im);

    imshow("Image cloned", result);
    waitKey(0);

    destx = desty = -1;
  }
}

int main(int argc, char **argv)
{
  string folder = "../images/";
  string id = "01";
  string original_path1 = folder + "source"      + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask"        + id + ".png";

  // read images
  Mat source = imread(original_path1);
  Mat destination = imread(original_path2);
  Mat mask = imread(original_path3);


  src = source;
  dst = destination;

  //////////// source image ///////////////////
  namedWindow("Source");
  setMouseCallback("Source", mouseHandler, NULL);
  imshow("Source", src);

  /////////// destination image ///////////////
  namedWindow("Destination");
  setMouseCallback("Destination", mouseHandler1, NULL);
  imshow("Destination", dst);

  waitKey(0);

  return 0;
}
