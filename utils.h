#ifndef __UTILS_H__
#define __UTILS_H__

#include <opencv2/highgui/highgui.hpp>

// print a statement
#define PRINT(X) (cout << #X": " << X << endl)

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

// Create an image where 'src' is paste into 'dst', starting in point 'p'
void cutpaste(const cv::Point &p, const cv::Mat &src, cv::Mat &dst);

#endif
