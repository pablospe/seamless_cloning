#include "path.h"
#include "interactive.h"

#include <iostream>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

// Some global variables (main() shouldn't know about these variables)
// TODO: remove these variables, probably using "void *param" in the handlers
static Mat g_src, g_dst, g_mask;
static Point g_point;
static int g_drag = 0;
static int destx = -1, desty = -1;

static void create_square_mask_handler(int event, int x, int y, int flags, void *param)
{
  if(event == CV_EVENT_LBUTTONDOWN && !g_drag)
  {
    g_point = Point(x, y);
    g_drag  = 1;
  }
  else if(event == CV_EVENT_LBUTTONDOWN && g_drag)
  {
    if(x - g_point.x < 0 || y - g_point.y < 0)
      return;

    Rect roi(g_point.x, g_point.y, x - g_point.x, y - g_point.y);
    Mat subimg;
    g_src(roi).copyTo(subimg);
    imshow("subimg", subimg);
    imshow("Source", g_src);
    waitKey(3);

    // create mask same size as source, and in white (255) the selected subimg
    g_mask  = Mat::zeros(g_src.size(), CV_8UC1);
    Mat white = Mat::ones(subimg.size(), CV_8UC1)*255;
    cutpaste(g_point, white, g_mask);

    g_drag = 0;
  }

  if(event == CV_EVENT_MOUSEMOVE && g_drag)
  {
    Mat tmp;
    g_src.copyTo(tmp);
    rectangle(tmp, g_point, Point(x, y), CV_RGB(255, 0, 0), 1, 8, 0);
    imshow("Source", tmp);
  }

  if(event == CV_EVENT_RBUTTONUP)
  {
    imshow("Source", g_src);
    g_drag = 0;
  }
}

void create_square_mask(const cv::Mat &src, cv::Mat &mask)
{
  g_point = Point(0,0);  // starting point of the ROI
  g_drag = 0;
  g_mask = Mat();
  g_src = src;

  namedWindow("Source");
  setMouseCallback("Source", create_square_mask_handler, NULL);
  imshow("Source", src);
  waitKey(0);

  g_mask.copyTo(mask);
}

static void place_src_handler(int event, int x, int y, int flags, void *param)
{
  Mat im;
  g_dst.copyTo(im);

  if(event == CV_EVENT_LBUTTONDOWN /*|| event == CV_EVENT_MOUSEMOVE*/)
  {
    destx = x;
    desty = y;

    cout << Point(x,y) << endl;

    // Draw mask bounding box (usually smaller than subimg box)
    Rect roi;
    get_bounding_box(g_mask, roi);
    rectangle(im, Point(x,y), Point(x + roi.width, y + roi.height), CV_RGB(0, 255, 255), 1, 8, 0);

    // Draw mask
    rectangle(im, Point(x - roi.x, y - roi.y), Point(x + g_mask.cols, y + g_mask.rows),
              CV_RGB(255, 0, 0), 1, 8, 0);

    if(x + roi.width > g_dst.cols || y + roi.height > g_dst.rows)
      cout << "Index out of range" << endl;

    imshow("Destination", im);
  }
}

void place_src(const cv::Mat &src,
               const cv::Mat &mask,
               const cv::Mat &dst,
               Point &dst_point)
{
  destx = desty = -1;
//   g_src  = src;
  g_mask = mask;
  g_dst  = dst;

  namedWindow("Destination");
  setMouseCallback("Destination", place_src_handler, NULL);
  imshow("Destination", dst);
  waitKey(0);

  dst_point = Point(destx, desty);
}


// TODO: delete these global variables
static Point prev_point = Point(-1,-1);
static Mat g_img;
static Path g_path;

static void on_mouse(int event, int x, int y, int flags, void *param)
{

  if(event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
  {
    g_point = Point(x, y);

    if( prev_point == Point(-1,-1) )
    {
      prev_point = g_point;
    }
    else
    {
      line(g_img, prev_point, g_point, CV_RGB(0, 255, 255), 2);
    }

    prev_point = g_point;
    g_path.push_back(g_point);
    imshow("Selection", g_img);
  }
}


void free_hand_selection(const Mat &img, Path &path)
{
  prev_point = Point(-1,-1);
  img.copyTo(g_img);
  g_path.clear();

  namedWindow("Selection");
  setMouseCallback("Selection", on_mouse, NULL);
  imshow("Selection", img);
  waitKey(0);

  path = g_path;
}
