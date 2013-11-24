#ifndef __INTERACTIVE_H__
#define __INTERACTIVE_H__

#include "utils.h"
#include "path.h"

//! \brief Select region of interest by left clicking with the mouse button.
//! A rectangular ROI (mask) will be created.
void create_square_mask(const cv::Mat &src, cv::Mat &mask);

//! \brief Select the place where src image will be in the final result.
void place_src(const cv::Mat &src,
               const cv::Mat &mask,
               const cv::Mat &dst,
               cv::Point &dst_point);

//! \brief Create a free hand mask.
void free_hand_selection(const cv::Mat &img, Path &path);

#endif // __INTERACTIVE_H__
