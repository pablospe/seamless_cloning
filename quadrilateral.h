#ifndef QUADRILATERAL_H
#define QUADRILATERAL_H

#include <opencv2/core/core.hpp>

/*! \brief Quadrilateral is a 4 points structure.
 *
 * Note: clockwise order is assumed, i.e.,
 *   - P[0] (left-top),
 *   - P[1] (right-top),
 *   - P[2] (right-bottom),
 *   - P[3] (left-bottom).
 */

class Quadrilateral
{
public:
  Quadrilateral();
  Quadrilateral(const Quadrilateral &other);
  Quadrilateral(const cv::Rect_<float> &R);
  Quadrilateral(const cv::Size &S);

  ~Quadrilateral();

  const Quadrilateral &operator=(const Quadrilateral &other);

  //! \brief Set quadrilateral points to cv::Rect R
  void fromRect(const cv::Rect_<float> &R);

  //! \brief Set quadrilateral points to cv::Size S (Rect(0, 0, S.width, S.height)
  void fromSize(const cv::Size &S);

  //! \brief Print quadrilateral
  void print();

  //! \brief Return the inner rectangle
  cv::Rect_<float> inner_rect();

  //! \brief Return the outer rectangle
  cv::Rect_<float> outer_rect();

  //! \brief Apply a perspective transform to the quadrilateral points
  // TODO: the relationship between points (left-top, right-top, etc.) might
  // change after this kind of transform. This should be checked.
  void apply_homography(const cv::Mat &H);

  // Getters
  cv::Point2f left_top()     const { return P_[0]; }
  cv::Point2f right_top()    const { return P_[1]; }
  cv::Point2f right_bottom() const { return P_[2]; }
  cv::Point2f left_bottom()  const { return P_[3]; }

  // Setters
  void set_left_top(const cv::Point2f &P)     { P_[0] = P;}
  void set_right_top(const cv::Point2f &P)    { P_[1] = P;}
  void set_right_bottom(const cv::Point2f &P) { P_[2] = P;}
  void set_left_bottom(const cv::Point2f &P)  { P_[3] = P;}

private:
  float left_min()   { return std::min(P_[0].x, P_[3].x); }
  float top_min()    { return std::min(P_[0].y, P_[1].y); }
  float right_min()  { return std::min(P_[1].x, P_[2].x); }
  float bottom_min() { return std::min(P_[3].y, P_[2].y); }

  float left_max()   { return std::max(P_[0].x, P_[3].x); }
  float top_max()    { return std::max(P_[0].y, P_[1].y); }
  float right_max()  { return std::max(P_[1].x, P_[2].x); }
  float bottom_max() { return std::max(P_[3].y, P_[2].y); }

private:
  std::vector<cv::Point2f> P_;
};

#endif // QUADRILATERAL_H
