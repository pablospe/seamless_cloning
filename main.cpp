#include <iostream>

#include "path.h"
#include "interactive.h"
#include "seamless_cloning.h"
#include "utils.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/core/types_c.h>

using namespace std;
using namespace cv;

cv::Mat findHomography(const cv::Mat &img_1,
                       const cv::Mat &img_2,
                       const cv::Mat &mask_1 =cv::Mat(),
                       const cv::Mat &mask_2 =cv::Mat())
{
  //! Detect the keypoints using a Detector (SIFT or SURF)
  SiftFeatureDetector detector;
//     SurfFeatureDetector detector;

  vector<KeyPoint> keypoints_1, keypoints_2;

  detector.detect( img_1, keypoints_1, mask_1 );
  detector.detect( img_2, keypoints_2, mask_2 );


  //! Calculate descriptors (feature vectors).
  SiftDescriptorExtractor extractor;
//     SurfDescriptorExtractor extractor;

  Mat descriptors_1, descriptors_2;
  extractor.compute( img_1, keypoints_1, descriptors_1 );
  extractor.compute( img_2, keypoints_2, descriptors_2 );


  //! Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  vector< DMatch > matches;
  matcher.match( descriptors_1, descriptors_2, matches );
  cout << "Number of matches = " << matches.size() << endl;

  double max_dist;
  double min_dist;

  min_dist = max_dist = matches[0].distance;

  //! Calculation of max and min distances between keypoints
  for( int i = 1; i < descriptors_1.rows; i++ )
  {
    double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }
  cout << " Max dist : " << max_dist << endl;
  cout << " Min dist : " << min_dist << endl;


  //! Get "good" matches (i.e. whose distance is less than certain threshold)
  vector<DMatch> good_matches;
  int threshold = 3;
  for( int i = 0; i < descriptors_1.rows; i++ )
  {
    if( matches[i].distance < threshold*min_dist )
    {
      good_matches.push_back( matches[i]);
    }
  }
  cout << "Number of good_matches = " << good_matches.size() << endl;

  if( good_matches.size() < 4 )
  {
    cerr << "error: good_matches.size() < 4\n";
    return Mat();
  }

  //! Localize the object from img_1 in img_2
  vector<Point2f> obj;
  vector<Point2f> scene;

  for( int i = 0; i < good_matches.size(); i++ )
  {
    // Get the keypoints from the good matches
    obj.push_back( keypoints_1[ good_matches[i].queryIdx ].pt );
    scene.push_back( keypoints_2[ good_matches[i].trainIdx ].pt );
  }

  //! FindHomography using Ransac
  vector<uchar> status;
  double ransacReprojThreshold=10;
  Mat H = findHomography( obj, scene, CV_RANSAC, ransacReprojThreshold, status );
  cout << "H = \n" << H << endl;

  //! Get inliers using the 'status' of the findHomography() function
  vector<DMatch> inliers;
  for( int i = 0; i < good_matches.size(); i++ )
  {
    if( status[i] )
    {
      inliers.push_back( good_matches[i] );
    }
  }
  cout << "Number of inliers = " << inliers.size() << endl;

//   //! Get the corners from the obj (the object to be "detected")
//   vector<Point2f> obj_corners(4);
//   obj_corners[0] = Point(0,0);
//   obj_corners[1] = Point( img_1.cols, 0 );
//   obj_corners[2] = Point( img_1.cols, img_1.rows );
//   obj_corners[3] = Point( 0, img_1.rows );
//   vector<Point2f> scene_corners(4);
//
//   perspectiveTransform( obj_corners, scene_corners, H );
//
//   //! Draw lines between the corners (the mapped object in the scene)
//   Mat img_matches;
//   drawMatches( img_1, keypoints_1, img_2, keypoints_2,
//                inliers, img_matches, Scalar::all(-1), Scalar::all(-1),
//                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
//
//   line( img_matches, scene_corners[0] + Point2f( img_1.cols, 0),
//         scene_corners[1] + Point2f( img_1.cols, 0),
//         Scalar(0, 255, 0), 2 );
//
//   line( img_matches, scene_corners[1] + Point2f( img_1.cols, 0),
//         scene_corners[2] + Point2f( img_1.cols, 0),
//         Scalar( 0, 255, 0), 2 );
//
//   line( img_matches, scene_corners[2] + Point2f( img_1.cols, 0),
//         scene_corners[3] + Point2f( img_1.cols, 0),
//         Scalar( 0, 255, 0), 2 );
//
//   line( img_matches, scene_corners[3] + Point2f( img_1.cols, 0),
//         scene_corners[0] + Point2f( img_1.cols, 0),
//         Scalar( 0, 255, 0), 2 );
//
//   //! Show detected matches
//   namedWindow( "Good Matches & Object detection", CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED );
//   imshow( "Good Matches & Object detection", img_matches );
//   waitKey(3);

  return H;
}


void on_click_mouse(int event, int x, int y, int flags, void *param)
{
  if(event == CV_EVENT_LBUTTONDOWN /*|| event == CV_EVENT_MOUSEMOVE*/)
    cout << Point(x,y) << endl;
}

int main(int argc, char **argv)
{
  string folder = "../images/";
  string id = "05";
  string original_path1 = folder + "source"      + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask"        + id + ".png";

  // read images
  Mat src  = imread(original_path1);
  Mat dst  = imread(original_path2);
  Mat mask = imread(original_path3);

  if(mask.channels() == 3)
    cvtColor(mask, mask, COLOR_BGR2GRAY);

  // Get homography
  Mat inverted_mask = 255 - mask;
  Mat H = findHomography(src, dst, inverted_mask);
  Mat H_inv = H.inv();


  //! Get the corners from the obj (the object to be "detected")
  vector<Point2f> obj_corners(4);
  obj_corners[0] = Point(0,0);
  obj_corners[1] = Point(dst.cols, 0);
  obj_corners[2] = Point(dst.cols, dst.rows);
  obj_corners[3] = Point(0, dst.rows);

  vector<Point2f> scene_corners(4);
  perspectiveTransform( obj_corners, scene_corners, H_inv );

  for(size_t i=0; i<scene_corners.size(); i++)
  {
    PRINT(scene_corners[i]);
  }


  vector<Point2f> P(4);
  P = scene_corners;
  float A, B, C, D;
  A = max(P[0].x,P[3].x);
  B = max(P[0].y,P[1].y);
  C = min(P[1].x,P[2].x);
  D = min(P[2].y,P[3].y);
  Rect_<float> roi = Rect_<float>( Point2f(A,B), Point2f(C,D) );
  PRINT(roi);

  Mat_<double> H_trans = (Mat_<double>(3,3) <<  1,  0, -A,
                                                0,  1, -B,
                                                0,  0,  1 );

  PRINT(H_trans);
  obj_corners[0] = Point2f(A,B);
  obj_corners[1] = Point2f(C,B);
  obj_corners[2] = Point2f(C,D);
  obj_corners[3] = Point2f(A,D);
  perspectiveTransform( obj_corners, scene_corners, H_trans );
  for(size_t i=0; i<scene_corners.size(); i++)
    PRINT(scene_corners[i]);



  // WarpPerspective: use homography to warp the image
  Mat dst_warped;
//   warpPerspective(dst, dst_warped, H.inv(), src.size());
//   PRINT(H_inv);

//   Mat_<float> H_trans = H;
//   H_trans(0,2) = H_trans(0,2) + scene_corners[0].x;
//   H_trans(1,2) = H_trans(1,2) + scene_corners[0].y;
//   H_trans(0,2) = 0;
//   H_trans(1,2) = 0;

//   PRINT(H_trans);
  PRINT(H_trans * H_inv);
//   PRINT(H_trans(1,2) = );

  Size size;
  size.width  = -A + C + 10; // min(-A + C, -A+src.cols)
  size.height = -B + D + 10; // min(-B + D, -B+src.rows)
  PRINT(size);
  PRINT(src.size());

  warpPerspective(dst, dst_warped, H_trans * H_inv, size);
  namedWindow("dst_warped");
  setMouseCallback("dst_warped", on_click_mouse, NULL);
  imshow("dst_warped", dst_warped );
  imshow("dst", dst);
//   waitKey(0);


  // Source & Mask (selected area)
  Mat src_mask;
  src.copyTo(src_mask, mask);
//   imshow("src_mask", src_mask );
//   imshow("src", src );

  // Cut & Paste
  Mat dst_copy = dst_warped.clone();
//   src_mask.copyTo(dst_copy, mask);
  src_mask.copyTo(dst_copy( Rect(Point(-A,-B), Point(-A+src.cols, -B+src.rows)) ), mask);
//   cutpaste(Point(-A,-B), src_mask, dst_copy);
  imshow("cutpaste", dst_copy );
  waitKey(500);

  // Seamless cloning
  Mat result;
  seamlessClone(src_mask, dst_warped, mask, Point(-A,-B), result, 1);
  imshow( "seamlessClone", result );

  Mat result_roi = result(Rect( Point(0,0), Point(-A+src.cols-10, -B+src.rows-10) ) );
  imshow( "result_roi", result_roi );

  imshow( "src", src );

  waitKey(0);

  return 0;
}



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
