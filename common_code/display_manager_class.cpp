#include "display_manager_class.h"
#include <assert.h>
#include <cassert>

display_manager::~display_manager(){}

display_manager::display_manager(int window_count=4):is_configured(false){
  if (window_count != 4) std::cout << "display manager constructor : currently only 4 windows are allowed" << std::endl;
  this->window_count = 4;
}

void display_manager::configure(cv::Mat template_image){

  // store the paramaters of the image that fits into a window
  window_rows = template_image.rows;
  window_cols = template_image.cols;
  output_image_type = template_image.type();

  // create the output window for 4 of the templates
  my_output_image = cv::Mat(window_rows*2, window_cols*2, output_image_type);
  std::cout << "display manager : configure : image rows/cols = " << window_rows << " , " << window_cols << " , type = " << output_image_type << std::endl;
  //std::cout << "display manager : configure : image size = " << my_image.size() << " , type = " << my_image.type() << std::endl;

  // create region of interest for the 4 winows in the outpout window for easy access later
  // beware of mat cv::image realworld image coordinates, ie. rows, cols, width, height and their mutations
  my_roi[0] = cv::Rect(0, 0, window_cols, window_rows);
  my_roi[1] = cv::Rect(window_cols, 0, window_cols, window_rows);
  my_roi[2] = cv::Rect(0, window_rows, window_cols, window_rows);
  //my_roi[2] = cv::Rect(0, window_cols, window_rows, window_cols);
  my_roi[3] = cv::Rect(window_cols, window_rows, window_cols, window_rows);

  std::cout << "rois" << std::endl;
  for (auto roi: my_roi){
    std::cout << roi << std::endl;
  }

  //assert (window_count=0);
  is_configured = true;
}


bool display_manager::set_window(std::string window_name, int window_position){
  if ( (window_position<0) | (window_position>=window_count) ) {
    std::cout << "display manager : set_window : position <0 or position >3" << std::endl;
    return false;
  }
  std::cout << "display manager : set_window : position : " << window_position << " -> " << window_name << std::endl;
  window_info[window_name] = window_position;
  return true;
}

bool display_manager::set_image(std::string window_name, cv::Mat image){

  //debug
  /*
  if (window_name == "hsv_filter") {
    std::cout << "display manager : set_image : testoutput for window name = " << window_name << std::endl;
    imshow("test hsv_filter",image);
  }  else std::cout << "display manager : set_image : window manager testoutput for all other windownames" << std::endl;
  */

  if (is_configured==false) {
    std::cout << "display manager : set_image : window manager not configured yet" << std::endl;
    return false;
  }

  int image_type = image.type();
  //std::cout << "display manager : set_image : image type = " << image_type << " target imagetype = " << output_image_type << std::endl;

  if (image_type != output_image_type) {
    std::cout << "display manager : set_image : converting image type from = " << image_type << " to " << output_image_type << std::endl;

    print_image_info(my_output_image,"expected");
    print_image_info(image,"image");

    if ( (image.channels() == 1 ) && (my_output_image.channels() ) == 3 ) {
      std::cout << "convert channels from 1 to 3 " << std::endl;
      cvtColor(image, image, cv::COLOR_GRAY2BGR);
    } else if ( (image.channels() == 3 ) & (my_output_image.channels() ) == 1 ) {
      std::cout << "convert channels from 3 to 1 " << std::endl;
      cvtColor(image, image, cv::COLOR_BGR2GRAY);
      std::cout << "seek cover, assuming you want BGR and not HSV/..." << std::endl;
    } else {
      std::cout << "conversions from " << image.channels() << " to " << my_output_image.channels() << " not yet implemented " << std::endl;
    }
    //cv::cvtColor(image,image, cv::COLOR_GRAY2BGR);
    // this produces a black image in case of small floats
    // image.convertTo(image, output_image_type);

    //imshow("before norm", image);
    cv::normalize( image, image, 0, 255, cv::NORM_MINMAX, -1, cv::Mat() );
    //imshow("after norm", image);
    image.convertTo( image, output_image_type);
    //imshow("after conversion", image);
    //print_image_info(image,"image");
  }

  int target_rows = window_rows;
  double row_factor = 1.0;
  if (image.rows != window_rows) {
    row_factor = (double) window_rows / (double) image.rows;
    std::cout << "required rows = " << window_rows << " found = " << image.rows << "   , factor = " << row_factor << std::endl;
  }

  int target_cols = window_cols;
  double col_factor = 1.0;
  if (image.cols != window_cols) {
    col_factor = (double) window_cols / (double) image.cols;
    std::cout << "required cols = " << window_cols << " found = " << image.cols << "   , factor = " << col_factor << std::endl;
  }

  if ( (row_factor!=1.0) | (col_factor!=1.0) ){
    std::cout << "adjusting image size, seek cover " << std::endl;
    cv::Mat resized_image;
    cv::Size target_size(window_cols,window_rows);
    cv::resize(image, image, target_size, cv::INTER_LINEAR);
  }

  // get the index of the output window from the name
  std::map<std::string,int>::const_iterator it = window_info.find(window_name);
  if ( it == window_info.end() ){
    std::cout << "display manager : set_image : window name = " << window_name << " not found in list" << std::endl;
    return false;
  }

  int window_position = it->second;
  std::cout << "display manager : set_image : " << window_name << " , pos = " << window_position << std::endl;

  // grab the associated roi to push the image into the window
  cv::Rect roi = my_roi[window_position];
  std::cout << "copy image rows/cols " << image.rows << "," << image.cols << " to window rows/cols " << my_output_image.rows << "," << my_output_image.cols << std::endl;
  std::cout << "roi " << roi << std::endl;

  //cv::Rect test_roi = cv::Rect(image.cols, image.rows, image.cols, image.rows);
  //std::cout << "roi " << roi << " , test_roi " << test_roi << std::endl;

  // most basic test to copy the image to the top left
  //imshow("image before roi copy", image);
  //cv::Rect test_roi = cv::Rect(0, 0, image.cols, image.rows);
  //std::cout << "roi " << roi << " , test_roi " << test_roi << std::endl;

  //cv::Rect test_roi = cv::Rect(image.cols, image.rows, image.cols, image.rows);
  cv::Mat insetImage(my_output_image, roi );
  //if (window_name == "hsv_filter") imshow("before roi copy", image);

  image.copyTo(insetImage);
  //imshow("after roi copy", my_output_image);

  //if (window_position == 2) imshow("inset",image);

  return true;
}

cv::Mat display_manager::get_image(){

  if (is_configured==false) {
    std::cout << "display manager : get_image : window manager not configured yet" << std::endl;
  }

  return my_output_image;
}
