#include <iostream>

#include "path.h"
#include "homography.h"
#include "interactive.h"
#include "seamless_cloning.h"
#include "utils.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/core/types_c.h>
// #include <opencv2/stitching/stitcher.hpp>

using namespace std;
using namespace cv;

void on_click_mouse(int event, int x, int y, int flags, void *param)
{
  if(event == CV_EVENT_LBUTTONDOWN /*|| event == CV_EVENT_MOUSEMOVE*/)
    cout << Point(x,y) << endl;
}

// Apply homography 'H' to corners of the image 'img'
void apply_H_to_corners(const Mat &img, const Mat &H, vector<Point2f> &projected_corners)
{
  int h = img.rows;
  int w = img.cols;

  // Get image corners and apply transformation
  vector<Point2f> corners(4);
  corners[0] = Point(0, 0);
  corners[1] = Point(w, 0);
  corners[2] = Point(w, h);
  corners[3] = Point(0, h);
  perspectiveTransform(corners, projected_corners, H);

//   for(size_t i = 0; i < projected_corners.size(); i++)
//     PRINT(projected_corners[i]);
}

// void quadri2rect()
// {}

int main(int argc, char **argv)
{
// /*

  // Command line
  const char *commandline_usage = "\tusage: ./seamless_cloning -h[--help] | --id <dataset>\n";
  string id = "03";

  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
    {
      cout << commandline_usage;
      return 0;
    }

    if (strcmp(argv[i], "--id") == 0)
    {
      if (argc > i + 1)
      {
        id = argv[i + 1];

      }
      else
      {
        cout << commandline_usage;
        return 0;
      }
    }
  }


  string folder = "../images/";
  string original_path1 = folder + "source"      + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask"        + id + ".png";

  // read images
  Mat src  = imread(original_path1);
  Mat dst  = imread(original_path2);

  Mat mask = imread(original_path3);
  if(mask.channels() == 3)
    cvtColor(mask, mask, COLOR_BGR2GRAY);

//   Mat mask;
//   free_hand_selection(src, mask);
//   imshow("mask", mask);
//   imwrite("mask.png", mask);
//   waitKey(0);

  // No mask
//   Mat mask = Mat::ones(src.size(), CV_8UC1);
//   cout << "01\n";

  // Find Homography
  Mat inverted_mask = 255 - mask;
  bool display = true;
//   bool display = false;
  Mat H = findH(src, dst, inverted_mask, cv::Mat(), display);
  Mat H_inv = H.inv();
//   waitKey(0);


  vector<Point2f> scene_corners(4);
  apply_H_to_corners(dst, H_inv, scene_corners);


  // Get inner rectangle from a quadrilateral
  vector<Point2f> P(4);
  P = scene_corners;
  float A, B, C, D;
  A = max(P[0].x, P[3].x);
  B = max(P[0].y, P[1].y);
  C = min(P[1].x, P[2].x);
  D = min(P[2].y, P[3].y);
  Rect_<float> roi = Rect_<float>(Point2f(A, B), Point2f(C, D));
  PRINT(roi);

  Mat_<double> H_trans = (Mat_<double>(3, 3) <<  1,  0, -A,
                                                 0,  1, -B,
                                                 0,  0,  1 );

  PRINT(H_trans);
  vector<Point2f> obj_corners(4);
  obj_corners[0] = Point2f(A, B);
  obj_corners[1] = Point2f(C, B);
  obj_corners[2] = Point2f(C, D);
  obj_corners[3] = Point2f(A, D);
  perspectiveTransform(obj_corners, scene_corners, H_trans);

  for(size_t i = 0; i < scene_corners.size(); i++)
    PRINT(scene_corners[i]);



//   PRINT(H_trans);
  PRINT(H_trans * H_inv);



  Size size;
//   size.width  = -A + C + 10; // min(-A + C, -A+src.cols)
//   size.height = -B + D + 10; // min(-B + D, -B+src.rows)
  size.width  = max(-A + C, -A+src.cols);
//   size.height = max(-B + D, -B+src.rows);
  size.height = min(-B + D, -B+src.rows);
  PRINT(size);
  PRINT(src.size());

  Mat dst_warped;
  warpPerspective(dst, dst_warped, H_trans * H_inv, size);
//   PRINT(H_inv);
  namedWindow("dst_warped");
  setMouseCallback("dst_warped", on_click_mouse, NULL);
//   imshow("dst_warped", dst_warped );
  imshow("dst", dst);
//   waitKey(0);


  // Reduced version of src and mask (common/visible area)
  Mat reduced_src, reduced_mask;
  roi = Rect( Point(0,0), Point(mask.cols, min(dst_warped.rows+(int)B, mask.rows) ) );
  reduced_mask = mask(roi).clone();
  reduced_src = src(roi).clone();
  reduced_src.copyTo(reduced_src, reduced_mask);

  imshow("reduced_src", reduced_src );
  imshow("src (Original)", src );
//   imshow("reduced_mask", reduced_mask );

  PRINT(reduced_src.size());
  PRINT(src.size());
//   waitKey(0);

  // Cut & Paste
  Mat dst_copy = dst_warped.clone();
  reduced_src.copyTo(dst_copy( Rect(Point(-A,-B), Point(-A+reduced_src.cols, -B+reduced_src.rows)) ), reduced_mask);
//   cutpaste(Point(-A,-B), reduced_src, dst_copy);
  imshow("cutpaste", dst_copy );
  waitKey(300);



  // Seamless cloning
  Mat result;
  seamlessClone(reduced_src, dst_warped, reduced_mask, Point(-A,-B), result, 1);
//   draw_cross(result, Point(-A,-B));
  imshow( "seamlessClone (Result)", result );

//   int offset = 10; // button offset
//   Mat result_roi = result(Rect(Point(0, 0), Point(result.cols, min(result.rows, src.rows - offset))) );
//   Mat result_roi = result(Rect(Point(0, 0), Point(result.cols, result.rows)));
//   imshow("result_roi", result_roi);
//   imshow("src (Original)", src);



  // Exit with 'q' or ESC (27)
  char k;
  while ( (k = waitKey(0)) != 'q' && k != 27 )
    ;

  return 0;

// */
}




/* OpenCV stitching
  string folder = "../images/";
  string id = "05";
  string original_path1 = folder + "source"      + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask"        + id + ".png";
  vector<Mat> imgs;
  Mat result;

  Stitcher stitcher = Stitcher::createDefault(); // The value you entered here is the default

  stitcher.setWarper(new PlaneWarper());

  imgs.push_back( imread(folder + "fede01_back01.jpg") );
  imgs.push_back( imread(folder + "fede01_back02.jpg") );
//   imgs.push_back( imread(folder + "fede01_back03.jpg") );

  Stitcher::Status status = stitcher.stitch(imgs, result);

  if (status != Stitcher::OK)
  {
      cout << "Error stitching - Code: " <<int(status)<<endl;
      return -1;
  }

//   imshow("Frame 1", imgs[0]);
//   imshow("Frame 2", imgs[1]);
  imshow("Stitched Image", result);
  imwrite(folder + "fede01_dst.png", result);
  waitKey();
  return 0;
*/



/* Square mask */
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



/* Free hand selection */

// //   Mat img = Mat::zeros(500,500,CV_8UC3);
//   Mat img = source;
//
//   // Create a free hand contour
//   Path path;
//   free_hand_selection(img, path);
//   for(size_t i=0; i<path.size(); i++)
//   {
//     PRINT(path[i]);
//   }
//
//   // Mask from contour
//   Mat mask = Mat::zeros(img.size(), CV_8UC1);
//   Path closed_path;
//   unsigned extent_to = mask.rows;
//   close_path(path, closed_path, extent_to);
//   contour2mask(closed_path, mask);
//   imshow("mask", mask);
//   waitKey(3);
//
//   // Contour from mask
//   Mat drawing = Mat::zeros(mask.size(), CV_8UC3);
//   Path new_path;
//   mask2contour(mask, new_path);
//   draw_path(mask, new_path, drawing);
//
//   imshow("drawing", drawing);
//   waitKey(0);
