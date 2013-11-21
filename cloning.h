#include "opencv2/core/core.hpp"

#ifndef __CLONING_H__
#define __CLONING_H__

class Cloning
{
public:
  void init_var(cv::Mat &I, cv::Mat &wmask);
  void initialization(cv::Mat &I, cv::Mat &mask, cv::Mat &wmask);
  void scalar_product(cv::Mat mat, float r, float g, float b);
  void array_product(cv::Mat mat1, cv::Mat mat2, cv::Mat mat3);
  void poisson(cv::Mat &I, cv::Mat &gx, cv::Mat &gy, cv::Mat &sx, cv::Mat &sy);
  void evaluate(cv::Mat &I, cv::Mat &wmask, cv::Mat &cloned);
  void getGradientx(const cv::Mat &img, cv::Mat &gx);
  void getGradienty(const cv::Mat &img, cv::Mat &gy);
  void lapx(const cv::Mat &img, cv::Mat &gxx);
  void lapy(const cv::Mat &img, cv::Mat &gyy);
  void dst(double *mod_diff, double *sineTransform, int h, int w);
  void idst(double *mod_diff, double *sineTransform, int h, int w);
  void transpose(double *mat, double *mat_t, int h, int w);
  void solve(const cv::Mat &img, double *mod_diff, cv::Mat &result);
  void poisson_solver(const cv::Mat &img, cv::Mat &gxx , cv::Mat &gyy, cv::Mat &result);
  void normal_clone(cv::Mat &I, cv::Mat &mask, cv::Mat &wmask, cv::Mat &cloned, int num);
  void local_color_change(cv::Mat &I, cv::Mat &mask, cv::Mat &wmask, cv::Mat &cloned, float red_mul, float green_mul, float blue_mul);
  void illum_change(cv::Mat &I, cv::Mat &mask, cv::Mat &wmask, cv::Mat &cloned, float alpha, float beta);
  void texture_flatten(cv::Mat &I, cv::Mat &mask, cv::Mat &wmask, double low_threshold, double high_threhold, int kernel_size, cv::Mat &cloned);

private:
  std::vector<cv::Mat> rgb_channel,
      rgbx_channel,
      rgby_channel,
      output;

  cv::Mat grx, gry, sgx, sgy, srx32, sry32, grx32, gry32, smask, smask1;
};

#endif
