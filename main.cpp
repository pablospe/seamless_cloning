#include <iostream>

#include "homography.h"
#include "interactive.h"
#include "path.h"
#include "quadrilateral.h"
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
// Note: clockwise order, starting with (0,0) -> (w,0) -> (w,h) -> (0,h)
void apply_H_to_corners(const Mat &img, const Mat &H,
                        vector<Point2f> &projected_corners)
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

Mat tmp;

void crop(const Rect &src,
          const Rect &mask,
          const Rect &inner,
          const Point &offset,
          Rect &roi)
{
  Rect left, right, top;

  left.x      = inner.x;
  left.y      = src.y;
  left.width  = src.x - inner.x;
  left.height = src.height;

  right.x      = src.x + src.width;
  right.y      = src.y;
  right.width  = inner.width - left.width - src.width;
  right.height = src.height;

  top.x      = src.x;
  top.y      = inner.y;
  top.width  = src.width;
  top.height = inner.height - src.height;

  // Debug
  Mat tmp2 = tmp.clone();
  draw_rect(tmp2,  src,  Scalar(0, 255, 255));
  draw_rect(tmp2, mask,  Scalar(0, 125, 255));
  draw_rect(tmp2, left,  Scalar(0,   0, 255));
  draw_rect(tmp2, right, Scalar(255, 0, 255));
  draw_rect(tmp2, top,   Scalar(255, 0,   0));
  imshow("tmp2", tmp2);

  draw_rect(tmp, src, Scalar(255, 0, 0));
  imshow("tmp", tmp);
  waitKey(0);

  roi = src;

  // Extend to left
  if(mask.x > src.x)
    roi |= left;

  draw_rect(tmp, roi, Scalar(0, 125, 255));
  imshow("tmp", tmp);
  waitKey(0);


  // Extend to right
  if(mask.x + mask.width < src.x + src.width)
    roi |= right;

  draw_rect(tmp, roi, Scalar(0, 0, 255));
  imshow("tmp", tmp);
  waitKey(0);


  // Extend to top
  if(mask.y > src.y)
    roi |= top;

  draw_rect(tmp, roi, Scalar(255, 0, 0));
  imshow("tmp", tmp);
  waitKey(0);
}

int main(int argc, char **argv)
{
// /*

  // Default dataset id
  string id = "04";

  // Command line
  const char *commandline_usage = "\tusage: ./seamless_cloning -h[--help] | --id <dataset>\n";
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

  // read mask
  Mat mask = imread(original_path3);
  if(mask.channels() == 3)
    cvtColor(mask, mask, COLOR_BGR2GRAY);

//   // create mask by hand
//   Mat mask;
//   free_hand_selection(src, mask);
//   imshow("mask", mask);
//   imwrite("mask.png", mask);
//   waitKey(0);

  // No mask
//  Mat mask(src.size(), CV_8UC1, 255); // TODO: there is a bug using '1' instead of '255'


  // Find Homography
  int max_level = *std::max_element(mask.begin<uchar>(), mask.end<uchar>());
  Mat inverted_mask = max_level - mask;
  bool display = true;
//   bool display = false;
  Mat H = findH(src, dst, inverted_mask, cv::Mat(), display);
  Mat H_inv = H.inv();


  // Get internal and external rectangle of a quadrilateral
  Quadrilateral dst_quad(dst.size());
  dst_quad.apply_homography(H_inv);
  Rect_<float> outer_rect = dst_quad.outer_rect();
  Rect_<float> inner_rect = dst_quad.inner_rect();

  // src ROI
  Rect_<float> src_roi = get_rect(src);

  // Union and Intersection ROIs
  Rect_<float> union_roi = src_roi | outer_rect;
  Rect_<float> inter_roi = src_roi & inner_rect;

  // mask ROI
  Rect mask_roi = get_bounding_box(mask);


  // H_trans: displacement
  Point offset;
  offset.x = -union_roi.x;
  offset.y = -union_roi.y;
  Mat_<double> H_trans = (Mat_<double>(3, 3) <<  1,  0, offset.x,
                                                 0,  1, offset.y,
                                                 0,  0,  1 );


  // Warp Perspective
  Size size(union_roi.width, union_roi.height);
  Mat dst_warped;
  warpPerspective(dst, dst_warped, H_trans * H_inv, size);
  imshow("dst_warped", dst_warped);
//   imshow("dst (back camera)", dst);
//   imwrite("dst.png", dst);


  // Reduced version of src and mask (common/visible area)
  Mat reduced_src, reduced_mask;
  Rect roi = inter_roi;
  reduced_mask = mask(roi).clone();
  src(roi).copyTo(reduced_src, reduced_mask);

  // The origin has been moved
  Point new_offset = offset;
  new_offset.x -= inter_roi.x;
  new_offset.y -= inter_roi.y;

  // Cut & Paste
  Mat cutpaste = dst_warped.clone();
  cut_and_paste(cutpaste, reduced_src, reduced_mask, new_offset);
  imshow("cutpaste", cutpaste);
//   imwrite("cutpaste.png", cutpaste );
  waitKey(300);


  // Seamless cloning
  Mat result;
  seamlessClone(reduced_src, dst_warped, reduced_mask, new_offset, result, 1);
//   draw_cross(result, new_offset);
  imshow( "seamlessClone (Result)", result );
//   imwrite("result.png", result);





  // Crop image, extend when possible (depending on the bounding box of the mask)
  tmp = result.clone();
  Rect reduced_src_roi  = get_rect(reduced_src);
  Rect reduced_mask_roi = get_bounding_box(reduced_mask);
  Rect inner_roi = inner_rect;
  apply_offset(reduced_src_roi,  new_offset);
  apply_offset(reduced_mask_roi, new_offset);
  apply_offset(inner_roi, offset);
  crop(reduced_src_roi, reduced_mask_roi, inner_roi, new_offset, roi);



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
