#include "opencv2/core/core.hpp"

void seamlessClone(cv::InputArray _src,
                   cv::InputArray _dst,
                   cv::InputArray _mask,
                   cv::Point p,
                   cv::OutputArray _blend,
                   int flags);

void colorChange(cv::InputArray _src,
                 cv::InputArray _mask,
                 cv::OutputArray _dst,
                 float r,
                 float g,
                 float b);

void illuminationChange(cv::InputArray _src,
                        cv::InputArray _mask,
                        cv::OutputArray _dst,
                        float a,
                        float b);

void textureFlattening(cv::InputArray _src,
                       cv::InputArray _mask,
                       cv::OutputArray _dst,
                       double low_threshold,
                       double high_threshold,
                       int kernel_size);
