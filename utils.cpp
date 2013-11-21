#include "utils.h"

#include <iostream>

using namespace std;
using namespace cv;

void cutpaste(const Point &p, const Mat &src, Mat &dst)
{
  if(p.x < 0 || p.y < 0 || p.x + src.cols > dst.cols || p.y + src.rows > dst.rows)
  {
    cout << "Out of range" << endl;
    return;
  }

  Mat dst_roi = dst(Rect(p.x, p.y, src.cols, src.rows));
  src.copyTo(dst_roi);
}
