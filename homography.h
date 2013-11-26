#ifndef __HOMOGRAPHY_H__
#define __HOMOGRAPHY_H__

#include <opencv2/core/core.hpp>

cv::Mat findH(const cv::Mat &img_1,
              const cv::Mat &img_2,
              const cv::Mat &mask_1 =cv::Mat(),
              const cv::Mat &mask_2 =cv::Mat(),
              bool display =false);

#endif // __HOMOGRAPHY_H__
