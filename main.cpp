#include <iostream>

#include "homography.h"
#include "interactive.h"
#include "path.h"
#include "quadrilateral.h"
#include "seamless_cloning.h"
#include "utils.h"

#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

// Command line parser
// usage: ./seamless_cloning -h[--help] | --id <dataset>
bool command_line_parser(int argc, char **argv, string &id);

// Read images
bool read_images(const string &id, Mat &src, Mat &dst, Mat &mask);

//
bool two_images(const Mat &src,
                const Mat &dst,
                const Mat &mask,
                Mat &result,
                Rect &roi,
                bool debug =false);

int main(int argc, char **argv)
{
  // Default dataset id
  string id = "07";

  // Command line
  bool status_ok = command_line_parser(argc, argv, id);
  if( !status_ok )
    return 1;

  // Read images
  Mat src, dst, mask;
  status_ok = read_images(id, src, dst, mask);
  if( !status_ok )
    return 1;

  // Apply pipeline
  Mat result;
  Rect roi;
  bool debug = true;
  status_ok = two_images(src, dst, mask, result, roi, debug);
  if( !status_ok )
    return 1;

  // Display final result, Press 'q' or ESC (27) to exit
  imshow("Final result", result(roi));
  char k;
  while ( (k = waitKey(0)) != 'q' && k != 27 )
    ;

  return 0;
}

bool command_line_parser(int argc, char **argv, string &id)
{
  const char *commandline_usage = "\tusage: ./seamless_cloning -h[--help] | --id <dataset>\n";

  for(int i = 1; i < argc; i++)
  {
    if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
    {
      cout << commandline_usage;
      return false;
    }

    if(strcmp(argv[i], "--id") == 0)
    {
      if(argc > i + 1)
      {
        id = argv[i + 1];
      }
      else
      {
        cout << commandline_usage;
        return false;
      }
    }
  }

  return true;
}

bool read_images(const string &id, Mat &src, Mat &dst, Mat &mask)
{
  string folder = "../images/";
  string original_path1 = folder + "source"      + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask"        + id + ".png";

  // read images
  src = imread(original_path1);
  dst = imread(original_path2);

  // read mask
  mask = imread(original_path3);
  if(mask.channels() == 3)
    cvtColor(mask, mask, COLOR_BGR2GRAY);

//   // create mask by hand
//   mask;
//   free_hand_selection(src, mask);
//   imshow("mask", mask);
//   imwrite("mask.png", mask);
//   waitKey(0);

  // No mask
//  mask = Mask(src.size(), CV_8UC1, 255); // TODO: there is a bug using '1' instead of '255'


  if(src.empty())
  {
    cerr << "Error loading images: " << original_path1 << endl;
    return false;
  }

  if(dst.empty())
  {
    cerr << "Error loading images: " << original_path2 << endl;
    return false;
  }

  if(mask.empty())
  {
    cerr << "Error loading images: " << original_path3 << endl;
    return false;
  }

  return true;
}

void crop(const Rect &src,
          const Rect &mask,
          const Rect &inner,
          const Point &offset,
          Rect &roi,
          Mat tmp =Mat())
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
  top.height = src.y - inner.y;

  // Debug
  if(!tmp.empty())
  {
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
  }

  roi = src;

  //! Extend to left
  if(mask.x > src.x)
    roi |= left;

  // Debug
  if(!tmp.empty())
  {
    draw_rect(tmp, roi, Scalar(0, 125, 255));
    imshow("tmp", tmp);
    waitKey(0);
  }


  //! Extend to right
  if(mask.x + mask.width < src.x + src.width)
    roi |= right;

  // Debug
  if(!tmp.empty())
  {
    draw_rect(tmp, roi, Scalar(0, 0, 255));
    imshow("tmp", tmp);
    waitKey(0);
  }


  //! Extend to top
  if(mask.y > src.y)
    roi |= top;

  // Debug
  if(!tmp.empty())
  {
    draw_rect(tmp, roi, Scalar(255, 0, 0));
    imshow("tmp", tmp);
    waitKey(0);
  }
}

bool two_images(const Mat &src,
                const Mat &dst,
                const Mat &mask,
                Mat &result,
                Rect &roi,
                bool debug)
{
  // Find Homography
  int max_level = *std::max_element(mask.begin<uchar>(), mask.end<uchar>());
  Mat inverted_mask = max_level - mask;
  bool display = debug;
  Mat H = findH(src, dst, inverted_mask, cv::Mat(), display);
  Mat H_inv = H.inv();

  // Get internal and external rectangle of a quadrilateral
  Quadrilateral dst_quad(dst.size());
  dst_quad.apply_homography(H_inv);
  Rect outer_rect = dst_quad.outer_rect();
  Rect inner_rect = dst_quad.inner_rect();

  // src ROI
  Rect src_roi = get_rect(src);

  // Union and Intersection ROIs
  Rect union_roi = src_roi | outer_rect;
  Rect inter_roi = src_roi & inner_rect;

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
  if(debug)
  {
    imshow("dst_warped (back-face camera)", dst_warped);
    imwrite("dst_warped.png", dst_warped);
    imwrite("dst.png", dst);
  }

  // Reduced version of src and mask (common/visible area)
  Mat reduced_src, reduced_mask;
  reduced_mask = mask(inter_roi).clone();
  src(inter_roi).copyTo(reduced_src, reduced_mask);

  // The origin has been moved
  Point new_offset = offset;
  new_offset.x -= inter_roi.x;
  new_offset.y -= inter_roi.y;

  // Cut & Paste
  Mat cutpaste = dst_warped.clone();
  cut_and_paste(cutpaste, reduced_src, reduced_mask, new_offset);
  if(debug)
  {
    imshow("cutpaste", cutpaste);
    imwrite("cutpaste.png", cutpaste );
    waitKey(300);
  }

  // Seamless cloning
  result;
  seamlessClone(reduced_src, dst_warped, reduced_mask, new_offset, result, 1);
  if(debug)
  {
//     draw_cross(result, new_offset);
    imshow("seamlessClone (Result)", result);
    imwrite("seamless_cloning.png", result);
  }

  // Get translated ROIs
  Rect reduced_src_roi  = get_rect(reduced_src);
  Rect reduced_mask_roi = get_bounding_box(reduced_mask);
  Rect inner_roi = inner_rect;
  apply_offset(reduced_src_roi,  new_offset);
  apply_offset(reduced_mask_roi, new_offset);
  apply_offset(inner_roi, offset); // 'offset'

  // Crop image, extend when possible (depending on the bounding box of the mask)
  Mat tmp;
  if(debug)
    tmp = result.clone();
  crop(reduced_src_roi, reduced_mask_roi, inner_roi, new_offset, roi, tmp);

  if(debug)
    imwrite("final_result.png", result(roi));

  return true;
}
