#ifndef __PATH_H__
#define __PATH_H__

#include <opencv2/highgui/highgui.hpp>

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

#endif // __PATH_H__
