#ifndef __UTILS_H__
#define __UTILS_H__

#include <opencv2/highgui/highgui.hpp>

// print a statement
#define PRINT(X) (cout << #X": " << X << endl)

//! \brief Draw a cross in the image.
inline void draw_cross(cv::Mat &img,
                       const cv::Point &center,
                       const cv::Scalar &color =cv::Scalar(0,0,255),
                       unsigned d =3)
{
  cv::line(img, cv::Point(center.x - d, center.y - d),
           cv::Point(center.x + d, center.y + d), color, 2, CV_AA, 0);
  cv::line(img, cv::Point(center.x + d, center.y - d),
           cv::Point(center.x - d, center.y + d), color, 2, CV_AA, 0);
}

//! \brief Create an image where 'src' is paste into 'dst', starting in point 'p'.
void cut_and_paste(cv::Mat &dst,
                   const cv::Mat &src,
                   const cv::Point &offset);

void cut_and_paste(cv::Mat &dst,
                   const cv::Mat &src,
                   const cv::Mat &mask,
                   const cv::Point &offset);

//! \brief Create a bounding box from a binary mask.
void get_bounding_box(const cv::Mat &mask, cv::Rect &roi);
inline cv::Rect get_bounding_box(const cv::Mat &mask)
{
  cv::Rect roi;
  get_bounding_box(mask, roi);
  return roi;
}

//! \brief Draw a cv::Rect
void draw_rect(cv::Mat &img,
               const cv::Rect &roi,
               const cv::Scalar &color =cv::Scalar(0, 255, 255));

//! \brief Get a cv::Rect from a cv::Mat
inline cv::Rect get_rect(const cv::Mat &img)
{
  return cv::Rect(0, 0, img.cols, img.rows);
}

inline cv::Rect apply_offset(cv::Rect &roi, const cv::Point &offset)
{
  roi.x += offset.x;
  roi.y += offset.y;
}

#endif // __UTILS_H__
