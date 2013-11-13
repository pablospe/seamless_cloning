#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "seamless_cloning.h"

using namespace std;
using namespace cv;

// #define STRINGIFY(x) #x
// #define TOSTRING(x) STRINGIFY(x)
#define PRINT(X) (cout << #X": " << X << endl)

Mat img0, img1, img2, subimg, result;

Point point;
int drag = 0;
int destx, desty;

void drawImage(Mat &target, const Mat &src, int x, int y)
{
  Mat dst_roi = target(Rect(x, y, src.cols, src.rows));
  src.copyTo(dst_roi);
}

void mouseHandler(int event, int x, int y, int flags, void *param)
{
  if(event == CV_EVENT_LBUTTONDOWN && !drag)
  {
    point = Point(x, y);
    drag  = 1;
  }

  if(event == CV_EVENT_MOUSEMOVE && drag)
  {
    img0.copyTo(img1);
    rectangle(img1, point, Point(x, y), CV_RGB(255, 0, 0), 1, 8, 0);
    imshow("Source", img1);
  }

  if(event == CV_EVENT_LBUTTONUP && drag)
  {
    img0.copyTo(img1);
    Rect roi(point.x, point.y, x - point.x, y - point.y);
    img1(roi).copyTo(subimg);
    imshow("ROI", subimg);
    waitKey(0);
    drag = 0;
  }

  if(event == CV_EVENT_RBUTTONUP)
  {
    imshow("Source", img0);
    drag = 0;
  }
}


void mouseHandler1(int event, int x, int y, int flags, void *param)
{
  Mat im;
  img2.copyTo(im);

  if(event == CV_EVENT_LBUTTONDOWN)
  {
    point = Point(x, y);

    rectangle(im, point, Point(x + subimg.cols, y + subimg.rows),
              CV_RGB(255, 0, 0), 1, 8, 0);

    destx = x;
    desty = y;

    cout << x << " " << y << " " << subimg.cols << " " << subimg.rows << " "
         << img2.cols << " " << img2.rows << endl;

    imshow("Destination", im);
  }

  if(event == CV_EVENT_RBUTTONUP)
  {

    if(destx + subimg.cols > img2.cols || desty + subimg.rows > img2.rows)
    {
      cout << "Index out of range" << endl;
      exit(0);
    }

    drawImage(im, subimg, destx, desty);
    Mat mask(subimg.size(), CV_8UC1, 255);
    seamlessClone(img0, img2, mask, Point(destx, desty), result, 1);

    ////////// save blended result ////////////////////
    imwrite("Output.jpg", result);
    imwrite("cutpaste.jpg", im);

    imshow("Image cloned", result);
    waitKey(0);
  }
}

int main(int argc, char **argv)
{
  if(argc < 3)
  {
    cerr << "Usage: " << argv[0] << " <source image> <destination image>\n";
    return 1;
  }

  img0 = imread(argv[1]);
  img2 = imread(argv[2]);

  //////////// source image ///////////////////
  namedWindow("Source");
  setMouseCallback("Source", mouseHandler, NULL);
  imshow("Source", img0);

  /////////// destination image ///////////////
  namedWindow("Destination");
  setMouseCallback("Destination", mouseHandler1, NULL);
  imshow("Destination", img2);

  waitKey(0);
  return 0;
}
