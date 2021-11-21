#include "support.h"

extern const std::vector<cv::Rect> init_view_windows;

#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"
//using namespace cv;

#include <iostream>

// global variables, necessary to make the image sliders work
//hsv parameters preset
int iLowH = 0;
int iHighH = 70;
int iLowS = 100;
int iHighS = 110, track_cone_high;
int iLowV = 80, track_cone_low;
int iHighV = 200;

// call back functions for the sliders, only necessary when they should ajdust / display something
static void tb_a_low( int, void* ){}
static void tb_a_high( int, void* ){}
static void tb_b_low( int, void* ){}
static void tb_b_high( int, void* ){}
static void tb_c_low( int, void* ){ track_cone_low = iLowV; }
static void tb_c_high( int, void* ){ track_cone_high = iHighS; }




int main(int, char**)
{
  // main window displays the original camera image and the sliders
  cv::namedWindow("original", cv::WINDOW_AUTOSIZE);

  //createTrackbar( variable name on slider , window , &variable, slider_max, callback function);
  //createTrackbar( "threshold", "original", &iLowH, 100, tb_a_low );
  //createTrackbar( "channel", "original", &iHighH, 3, tb_a_high );
  //createTrackbar( "background_high", "original", &iHighH, 255, tb_b_high );
  //createTrackbar( "background_low", "original", &iLowS, 255, tb_b_low );
	//createTrackbar( "cone_high", "original", &iHighS, 255, tb_c_high );
	//createTrackbar( "cone_low", "original", &iLowV, 255, tb_c_low );

  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // read the list with cone patterns, one will be used to find the cone in the image
  std::filesystem::path cone_pattern_dir_path = "../../cone_templates";
  std::vector<path> pattern_list;
  pattern_list = read_image_list(cone_pattern_dir_path);

  // opencv global variables
  cv::Mat frame, display_image, cone_template, pre_processed_frame;

  // index for the images in the video and index for the various cone template images
  int frame_counter = 0;
  int pattern_counter = 0;

  // define which mode the images are read from file
  // for this app we work with grayscale
  cv::ImreadModes image_mode = cv::IMREAD_COLOR; //;//IMREAD_GRAYSCALE; //

  cv::namedWindow("main", cv::WINDOW_AUTOSIZE);
  display_manager window_man(4);

  spider_web_class web;

  cv::Rect init_roi;

  // until press escape
  while(1){

    // read and display the current cone pattern
    path pattern_path = pattern_list.at(pattern_counter);
    //cv::cone_template = imread(pattern_path, image_mode );
    cone_template = create_cone_template();
    //cv::cone_template =  cv::Scalar::all(255) - cone_template;
    //cv::imshow("template", cone_template);

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    std::cout << "loading image " << frame_counter << std::endl;
    frame = imread(image_path, image_mode );
    display_image = frame.clone();

    if (window_man.is_configured==false) {
      window_man.configure(frame);
      window_man.set_window("original",0);
      window_man.set_window("filter",1);
      window_man.set_window("match",2);
      window_man.set_window("result",3);

      init_roi = cv::Rect(0,200,frame.cols,125);

      //web.create_web(10, 10, 100, 100, 30);
      //web.create_web(10, 10, frame.cols - 10, frame.rows - 10, 10);
      web.create_web(init_roi.x, init_roi.y, init_roi.x + init_roi.width, init_roi.y + init_roi.height,10);

    }

    cv::Mat imgHSV, threshold_output;
    cv::cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV);

    //print_image_info(frame,"frame");
    //cv::waitKey(0);

    cv::Point current = web.restart();
    std::cout << "starting with " << current << std::endl;
    while (current != cv::Point()){
      //std::cout << "circling " << current << std::endl;
      cv::Vec3b color = imgHSV.at<cv::Vec3b>(current);
      //std::cout << "color " << color << std::endl;

      if (is_orange(color)) {
        cv::circle(display_image, current, 5, cv::Scalar(0,0,255), 1);
      } else {
        cv::circle(display_image, current, 3, cv::Scalar(255,255,255), 1);
      }
      current = web.get_next();
    }

    cv::rectangle(
      frame,
      init_roi,
      cv::Scalar(0, 0, 255),1, 8, 0
    );

    imshow("original", frame);
    window_man.set_image("result", frame);
    window_man.set_image("filter", display_image);
    cv::Mat fucker = window_man.get_image();
    imshow("main",fucker);

    // wait for button press
    // escape = end
    // space = next frame
    // meanwhile use the sliders
    char c = (char) cv::waitKey(25);

    // esc ends the show
    if(c==27) // esc
      break;
    // next frame
    else if (c==32) {
      if (frame_counter < image_list.size()) frame_counter++; else frame_counter = 0;
    }
  }

  return 0;
}
