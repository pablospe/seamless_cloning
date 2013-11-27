#include "homography.h"

#include <iostream>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

using namespace cv;
using namespace std;

cv::Mat findH(const cv::Mat &img_1,
              const cv::Mat &img_2,
              const cv::Mat &mask_1,
              const cv::Mat &mask_2,
              bool display)
{
  // Detect the keypoints using a Detector (SIFT or SURF)
  SiftFeatureDetector detector;  // or SurfFeatureDetector
  vector<KeyPoint> keypoints_1, keypoints_2;
  detector.detect(img_1, keypoints_1);
  detector.detect(img_2, keypoints_2);

  // Calculate descriptors (feature vectors).
  SiftDescriptorExtractor extractor;  // or SurfDescriptorExtractor
  Mat descriptors_1, descriptors_2;
  extractor.compute(img_1, keypoints_1, descriptors_1);
  extractor.compute(img_2, keypoints_2, descriptors_2);

  // Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  vector< DMatch > matches;
  matcher.match(descriptors_1, descriptors_2, matches);
  cout << "Number of matches = " << matches.size() << endl;

  // Calculation of max and min distances between keypoints
  double max_dist;
  double min_dist;
  min_dist = max_dist = matches[0].distance;
  for(int i = 1; i < descriptors_1.rows; i++)
  {
    double dist = matches[i].distance;

    if(dist < min_dist) min_dist = dist;

    if(dist > max_dist) max_dist = dist;
  }
  cout << " Max dist : " << max_dist << endl;
  cout << " Min dist : " << min_dist << endl;

  // Get "good" matches (i.e. whose distance is less than certain threshold)
  vector<DMatch> good_matches;
  int threshold = 3;
//   int threshold = 5;
  for(int i = 0; i < descriptors_1.rows; i++)
  {
    if(matches[i].distance < threshold * min_dist)
    {
      good_matches.push_back(matches[i]);
    }
  }
  cout << "Number of good_matches = " << good_matches.size() << endl;

  if(good_matches.size() < 4)
  {
    cerr << "error: good_matches.size() < 4\n";
    return Mat();
  }

  // Localize the object from img_1 in img_2
  vector<Point2f> obj;
  vector<Point2f> scene;
  for(int i = 0; i < good_matches.size(); i++)
  {
    // Get the keypoints from the good matches
    obj.push_back(keypoints_1[ good_matches[i].queryIdx ].pt);
    scene.push_back(keypoints_2[ good_matches[i].trainIdx ].pt);
  }

  // FindHomography using Ransac
  vector<uchar> status;
  double ransacReprojThreshold = 10;
  Mat H = findHomography(obj, scene, CV_RANSAC, ransacReprojThreshold, status);
  cout << "H = \n" << H << endl;

  if(display)
  {
    // Get inliers using the 'status' of the findHomography() function
    vector<DMatch> inliers;

    for(int i = 0; i < good_matches.size(); i++)
    {
      if(status[i])
      {
        inliers.push_back(good_matches[i]);
      }
    }

    cout << "Number of inliers = " << inliers.size() << endl;

    // Get the corners from the obj (the object to be "detected")
    vector<Point2f> obj_corners(4);
    obj_corners[0] = Point(0, 0);
    obj_corners[1] = Point(img_1.cols, 0);
    obj_corners[2] = Point(img_1.cols, img_1.rows);
    obj_corners[3] = Point(0, img_1.rows);
    vector<Point2f> scene_corners(4);

    perspectiveTransform(obj_corners, scene_corners, H);

    // Draw lines between the corners (the mapped object in the scene)
    Mat img_matches;
    drawMatches(img_1, keypoints_1, img_2, keypoints_2,
                inliers, img_matches, Scalar::all(-1), Scalar::all(-1),
                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    line(img_matches, scene_corners[0] + Point2f(img_1.cols, 0),
        scene_corners[1] + Point2f(img_1.cols, 0),
        Scalar(0, 255, 0), 2);

    line(img_matches, scene_corners[1] + Point2f(img_1.cols, 0),
        scene_corners[2] + Point2f(img_1.cols, 0),
        Scalar(0, 255, 0), 2);

    line(img_matches, scene_corners[2] + Point2f(img_1.cols, 0),
        scene_corners[3] + Point2f(img_1.cols, 0),
        Scalar(0, 255, 0), 2);

    line(img_matches, scene_corners[3] + Point2f(img_1.cols, 0),
        scene_corners[0] + Point2f(img_1.cols, 0),
        Scalar(0, 255, 0), 2);

    // Show detected matches
//     namedWindow("Good Matches & Object detection", CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
//     imshow("homography", img_matches);
    imwrite("homography.png", img_matches);
//     waitKey(3);
  }

  return H;
}
