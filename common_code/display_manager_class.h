#ifndef DISPLAY_MAN
#define DISPLAY_MAN

#include <iostream>
#include "opencv2/opencv.hpp"
#include "image_operations.h"
#include <string>
#include <map>

class display_manager{

public:
  display_manager(int window_count);
  ~display_manager();

  void configure(cv::Mat template_image);
  bool set_window(std::string window_name, int window_position);
  bool set_image(std::string window_name, cv::Mat image);
  cv::Mat get_image();

  bool is_configured;
  void toggle_debug_output(bool onoff);

private:
  // window count: number of windows in the output ,currently fixed to 4
  // window_rows and window_cols: size of images which will be displayed in the 4 windows
  // window_type: type of image (color, gray, int, float etc)
  int window_count, window_rows, window_cols;

  // maps the name of the output window to an integer index
  std::map<std::string,int> window_info;

  // predefined regions of interests to get easy access to the 4 windows inthe output image
  cv::Rect my_roi[4];

  // the output image
  cv::Mat my_output_image;
  int output_image_type;

  bool debug_on;

};

#endif
