#include <iomanip>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

#include "interactive.h"
#include "seamless_cloning.h"

using namespace std;
using namespace cv;

// Command line parser
// usage: ./seamless_cloning -h[--help] | --id <dataset>
bool command_line_parser(int argc, char **argv, string &id);

// Read images
bool read_images(const string &id, Mat &src, Mat &dst, Mat &mask);

int main(int argc, char **argv) {
  // Default dataset id
  string id = "01";

  // Command line
  //   bool status_ok = command_line_parser(argc, argv, id);
  //   if( !status_ok )
  //     return 1;

  // Read images
  Mat src, dst, mask;
  bool status_ok = read_images(id, src, dst, mask);
  if (!status_ok) return 1;

  // No mask
  //   mask = Mat(src.size(), CV_8UC1, 255);

  // Seamless cloning
  Point offset;
  offset.x = 300;
  offset.y = 5;
  Mat result;

  seamlessClone(src, dst, mask, offset, result, 1);
  imshow("seamlessClone (src)", src);
  imshow("seamlessClone (dst)", dst);
  imshow("seamlessClone (mask)", mask);
  imshow("seamlessClone (Result)", result);

  // wait until 'q'
  for (char k; (k = waitKey(0)) != 'q' && k != 27;)
    ;

  imwrite("seamless_cloning.png", result);

  return 0;
}

bool command_line_parser(int argc, char **argv, string &id) {
  const char *commandline_usage =
      "\tusage: ./seamless_cloning -h[--help] | --id <dataset>\n";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      cout << commandline_usage;
      return false;
    }

    if (strcmp(argv[i], "--id") == 0) {
      if (argc > i + 1) {
        id = argv[i + 1];
      } else {
        cout << commandline_usage;
        return false;
      }
    }
  }

  return true;
}

bool read_images(const string &id, Mat &src, Mat &dst, Mat &mask) {
  string folder = "../images/";
  string original_path1 = folder + "source" + id + ".png";
  string original_path2 = folder + "destination" + id + ".png";
  string original_path3 = folder + "mask" + id + ".png";

  // read images
  src = imread(original_path1);
  dst = imread(original_path2);

  // read mask
  mask = imread(original_path3);
  if (mask.channels() == 3) cvtColor(mask, mask, COLOR_BGR2GRAY);

  // create mask by hand
  if (mask.empty()) {
    free_hand_selection(src, mask);
    imshow("mask", mask);
    imwrite("mask.png", mask);
    waitKey(0);
  }

  // No mask. TODO: there is a bug using '1'  instead of '255'
  //  mask = Mask(src.size(), CV_8UC1, 255);

  if (src.empty()) {
    cerr << "Error loading image: " << original_path1 << endl;
    return false;
  }

  if (dst.empty()) {
    cerr << "Error loading image: " << original_path2 << endl;
    return false;
  }

  if (mask.empty()) {
    cerr << "Error loading image: " << original_path3 << endl;
    return false;
  }

  return true;
}
