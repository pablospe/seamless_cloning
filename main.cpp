#include <iostream>
#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>

#include "seamless_cloning.h"
#include "utils.h"

using namespace std;
using namespace cv;

/* Select region of interest by left click mouse button.
 * A rectangular ROI (mask) will be created */
void create_square_mask(const cv::Mat &src, cv::Mat &mask);

/* Mouse handler for create_square_mask(...) */
void create_square_mask_handler(int event, int x, int y, int flags, void *param);


/* Select the place where src image will be in the final result */
void place_src(const cv::Mat &src,
               const cv::Mat &mask,
               const cv::Mat &dst,
               Point &dst_point);

/* Mouse handler for place_src_handler(...) */
void place_src_handler(int event, int x, int y, int flags, void *param);


int main(int argc, char **argv)
{
  string folder = "../images/";
  string id = "02";
  string original_path1 = folder + "source"      + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask"        + id + ".png";

  // read images
  Mat source = imread(original_path1);
  Mat destination = imread(original_path2);
//   Mat mask = imread(original_path3);

  // source image
  Mat mask;
  create_square_mask(source, mask);

  // destination image
  Point dst_point;
  place_src(source, mask, destination, dst_point);

  Mat result;
  seamlessClone(source, destination, mask, dst_point, result, 1);

  // save blended result
  imshow("Image cloned", result);
//   imwrite("result.png", result);
  waitKey(0);

  return 0;
}



// Some global variables (main() shouldn't know about these variables)
// TODO: remove these variables, probably using "void *param" in the handlers
Mat g_src, g_dst, subimg, result, g_mask;
Point point;
int drag = 0;
int destx = -1, desty = -1;


// Note: it will change some global variables: point, drag, subimg, mask
void create_square_mask_handler(int event, int x, int y, int flags, void *param)
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
    g_src(roi).copyTo(subimg);
    imshow("subimg", subimg);
    imshow("Source", g_src);
    waitKey(3);

    // create mask same size as source, and in white (255) the selected subimg
    g_mask  = Mat::zeros(g_src.size(), CV_8UC1);
    Mat white = Mat::ones(subimg.size(), CV_8UC1)*255;
    cutpaste(point, white, g_mask);

    drag = 0;
  }

  if(event == CV_EVENT_MOUSEMOVE && drag)
  {
    Mat tmp;
    g_src.copyTo(tmp);
    rectangle(tmp, point, Point(x, y), CV_RGB(255, 0, 0), 1, 8, 0);
    imshow("Source", tmp);
  }

  if(event == CV_EVENT_RBUTTONUP)
  {
    imshow("Source", g_src);
    drag = 0;
  }
}

void create_square_mask(const cv::Mat &src, cv::Mat &mask)
{
  // TODO: don't use global variables
  point = Point(0,0);  // starting point of the ROI
  drag = 0;
  subimg = Mat();
  g_mask = Mat();

  g_src = src;

  namedWindow("Source");
  setMouseCallback("Source", create_square_mask_handler, NULL);
  imshow("Source", src);
  waitKey(0);

  g_mask.copyTo(mask);
}

void place_src_handler(int event, int x, int y, int flags, void *param)
{
  Mat im;
  g_dst.copyTo(im);

  if(event == CV_EVENT_LBUTTONDOWN)
  {
    rectangle(im, Point(x, y), Point(x + subimg.cols, y + subimg.rows),
              CV_RGB(255, 0, 0), 1, 8, 0);

    destx = x;
    desty = y;

    cout << Point(x, y) << endl;

    imshow("Destination", im);
  }
}

void place_src(const cv::Mat &src,
               const cv::Mat &mask,
               const cv::Mat &dst,
               Point &dst_point)
{
  // TODO: don't use global variables
  destx = desty = -1;

  g_src  = src;
  g_mask = mask;
  g_dst  = dst;

  namedWindow("Destination");
  setMouseCallback("Destination", place_src_handler, NULL);
  imshow("Destination", dst);
  waitKey(0);

  dst_point = Point(destx, desty);
}
