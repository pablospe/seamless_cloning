#include <iostream>

#include "path.h"
#include "interactive.h"
#include "seamless_cloning.h"
#include "utils.h"

using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
  string folder = "../images/";
  string id = "05";
  string original_path1 = folder + "source"      + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask"        + id + ".png";

  // read images
  Mat source = imread(original_path1);
  Mat destination = imread(original_path2);
//   Mat mask = imread(original_path3);

//   // source image
//   Mat mask;
//   create_square_mask(source, mask);
//
//   // destination image
//   Point dst_point;
//   place_src(source, mask, destination, dst_point);
//
//   Mat result;
//   seamlessClone(source, destination, mask, dst_point, result, 1);
//
//   // save blended result
//   imshow("Image cloned", result);
// //   imwrite("result.png", result);
//   waitKey(0);
//   return 0;


//   Mat img = Mat::zeros(500,500,CV_8UC3);
  Mat img = source;

  // Create a free hand contour
  Path path;
  free_hand_selection(img, path);
  for(size_t i=0; i<path.size(); i++)
  {
    PRINT(path[i]);
  }

  // Mask from contour
  Mat mask = Mat::zeros(img.size(), CV_8UC1);
  Path closed_path;
  unsigned extent_to = mask.rows;
  close_path(path, closed_path, extent_to);
  contour2mask(closed_path, mask);
  imshow("mask", mask);
  waitKey(3);

  // Contour from mask
  Mat drawing = Mat::zeros(mask.size(), CV_8UC3);
  Path new_path;
  mask2contour(mask, new_path);
  draw_path(mask, new_path, drawing);

  imshow("drawing", drawing);
  waitKey(0);

  return 0;
}




