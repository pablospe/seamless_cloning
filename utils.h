#ifndef __UTILS_H__
#define __UTILS_H__

#include <opencv2/highgui/highgui.hpp>

// print a statement
#define PRINT(X) (cout << #X": " << X << endl)

//! \brief Draw a cross in the image.
inline void draw_cross(cv::Mat &img,
                       const cv::Point &center,
                       const cv::Scalar &color,
                       unsigned d)
{
  cv::line(img, cv::Point(center.x - d, center.y - d),
           cv::Point(center.x + d, center.y + d), color, 2, CV_AA, 0);
  cv::line(img, cv::Point(center.x + d, center.y - d),
           cv::Point(center.x - d, center.y + d), color, 2, CV_AA, 0);
}

//! \brief Create an image where 'src' is paste into 'dst', starting in point 'p'.
void cutpaste(const cv::Point &p, const cv::Mat &src, cv::Mat &dst);

//! \brief Create a bounding box from a binary mask.
void get_bounding_box(const cv::Mat &mask, cv::Rect &roi);

/**
 *   Contour/Path utils
 */

typedef std::vector<cv::Point> Path;

//! \brief Create a mask from a closed path (or contour).
void contour2mask(const Path &path, cv::Mat &mask);

//! \brief Try to find the contour. It is assumed that there is only one.
void mask2contour(const cv::Mat &mask, Path &contour);

//! \brief Draw a path in the image 'out'.
void draw_path(const cv::Mat &in, Path &path, cv::Mat &out,
               const cv::Scalar &color =CV_RGB(0, 255, 125));

//! \brief Close an open path (first and last element will be the same).
//! It is possible to extend to the image bottom with 'extend_to'.
void close_path(const Path &path, Path &closed_path, unsigned extend_to =0);

#endif // __UTILS_H__
