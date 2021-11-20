#include <iostream>

#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"
using namespace cv;

//hsv parameters preset
int hsv_hue_low = 0;//0
int hsv_hue_high = 70;//192
int hsv_sat_low = 100;//0
int hsv_sat_high = 255;//145
int hsv_value_low = 70; //5
int hsv_value_high = 200; //159

// rgb parameters preset
/*
int iLowH = 15;
int iHighH = 52;
int iLowS = 0;
int iHighS = 255;
int iLowV = 75;
int iHighV = 175;
*/

// call back functions for the sliders, only necessary when they should ajdust / display something
static void callback_hsv_hue_low( int, void* ){}
static void callback_hsv_hue_high( int, void* ){}
static void callback_hsv_sat_low( int, void* ){}
static void callback_hsv_sat_high( int, void* ){}
static void callback_hsv_value_low( int, void* ){}
static void callback_hsv_value_high( int, void* ){}


int main(int, char**)
{
  // main window displays the original camera image and the sliders
  namedWindow("original", WINDOW_AUTOSIZE);
  //createTrackbar( "threshold", "original", &iLowH, 100, tb_a_low );

  // channel, max is 2, where used in code verify that setting is permissible (i.e. < channels())
  //createTrackbar( "channel", "original", &iHighH, 3, tb_a_high );

  /*
  createTrackbar( "r_high", "original", &iHighH, slider_max, tb_a_high );
  createTrackbar( "g_low", "original", &iLowS, slider_max, tb_a_low );
	createTrackbar( "g_high", "original", &iHighS, slider_max, tb_a_high );
	createTrackbar( "b_low", "original", &iLowV, slider_max, tb_a_low );
	createTrackbar( "b_high", "original", &iHighV, slider_max, tb_a_high );
  */
  int slider_max = 255;
  createTrackbar( "hsv_hue_low", "original", &hsv_hue_low, slider_max, callback_hsv_hue_low );
  createTrackbar( "hsv_hue_high", "original", &hsv_hue_high, slider_max, callback_hsv_hue_high );
  createTrackbar( "hsv_sat_low", "original", &hsv_sat_low, slider_max, callback_hsv_sat_low );
  createTrackbar( "hsv_sat_high", "original", &hsv_sat_high, slider_max, callback_hsv_sat_high );
  createTrackbar( "hsv_value_low", "original", &hsv_value_low, slider_max, callback_hsv_value_low );
  createTrackbar( "hsv_value_high", "original", &hsv_value_high, slider_max, callback_hsv_value_high );


  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // read the list with cone patterns, one will be used to find the cone in the image
  std::filesystem::path cone_pattern_dir_path = "../../cone_templates";
  std::vector<path> pattern_list;
  pattern_list = read_image_list(cone_pattern_dir_path);

  namedWindow("main", WINDOW_AUTOSIZE);
  display_manager window_man(4);

  // opencv global variables
  Mat frame, cone_template;

  int frame_counter = 0;
  int pattern_counter = 0;

  ImreadModes image_mode = IMREAD_COLOR;//IMREAD_GRAYSCALE; //

  // until press escape
  while(1){

    // read and display the current cone pattern
    path pattern_path = pattern_list.at(pattern_counter);
    //cone_template = imread(pattern_path, image_mode );

    cone_template = create_cone_template();

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    frame = imread(image_path, image_mode );
    imshow("original", frame);
    window_man.set_image("original", frame  );

    cv::Mat small;
    double scale_factor = 1.0/1.0;
    resize(frame, small, cv::Size(), scale_factor, scale_factor, cv::INTER_LINEAR);

    if (window_man.is_configured==false) {
      window_man.configure(frame);
      window_man.set_window("original",0);
      window_man.set_window("hsv_filter",1);
      window_man.set_window("contours",2);
      window_man.set_window("result",3);
    }

    // filter orange color
    cv::Scalar lower_bound = cv::Scalar(hsv_hue_low, hsv_sat_low, hsv_value_low);
    cv::Scalar upper_bound = cv::Scalar(hsv_hue_high, hsv_sat_high, hsv_value_high);
    cv::Mat orange_mask = filter_hsv_for_color(small, lower_bound, upper_bound);
    window_man.set_image("hsv_filter", orange_mask);
    //imshow("hsv_filter", orange_mask);

    //cv::Mat keep_organge_mask = orange_mask.clone();

    // find the contours in the binary image
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    // RETR_LIST : child / parent levels omitted, ie. the hierarchy contains all contours with no inner / outer relationship
    findContours(orange_mask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));


    //cv::RETR_TREE, RETR_LIST, RETR_CCOMP
    //cv::CHAIN_APPROX_SIMPLE,cv::RETR_EXTERNAL,

    // draw the contours

    cv::Mat result = small.clone();

    cv::Mat contour_frame = cv::Mat::zeros(orange_mask.rows, orange_mask.cols, CV_8UC1);//(keep_organge_mask.size(), CV_8UC1);

    for (size_t k = 0; k < contours.size(); ++k)
    {

      std::vector<cv::Point> hull_points(contours.size());

      int next_contour = hierarchy[k][0];
      int prev_contour = hierarchy[k][1];
      int child_contour = hierarchy[k][2];
      int parent_contour = hierarchy[k][3];

      if ( (child_contour < 0) & (parent_contour < 0) )
        cv::drawContours(contour_frame,contours,k,128);
      else
        cv::drawContours(contour_frame,contours,k,255);

        cv::convexHull(contours[k], hull_points);//, False);

        std::vector<std::vector<cv::Point>> hull_result;
        hull_result.push_back(hull_points);
        //cv::drawContours(result,hull_result,0,cv::Scalar(128,128,255));

        std::vector<cv::Point> approx;
        approxPolyDP(contours[k], approx, arcLength(contours[k], true)*0.02, true);
        bool ok = (fabs(contourArea(approx)) > 50) && (fabs(contourArea(approx)) < 6000);// && (isContourConvex(approx) );

        if (ok) {
          hull_result.clear();
          hull_result.push_back(approx);
          cv::drawContours(result,hull_result,0,cv::Scalar(255,128,0));

          //std::cout << "area found = " << fabs(contourArea(approx)) << std::endl;
        }

      int contour_count = contours.size();
      //std::cout << "contours found = " << contour_count << std::endl;

      if ( (child_contour > -1) & (parent_contour >-1) ) {
        //std::cout << "fucked up, hierarierchy child/parents set for retr_list" << std::endl;
      }

      //std::cout << "contour = " << k << "   next = " << next_contour << "   prev = " << prev_contour << "   child = " << child_contour << "   parent = " << parent_contour << std::endl;

      //
      /*
      int count = hull_points.size();
      std::cout << "found = " << count << std::endl;
      if (count>2)
        cv::polylines( result, hull_points, true, Scalar(0,0,255), 2 );
        */
  }

    window_man.set_image("contours", contour_frame);

    window_man.set_image("result", result);
    cv::Mat fucker = window_man.get_image();
    imshow("main",fucker);

    // wait for button press
    // escape = end
    // space = next frame
    // meanwhile use the sliders
    char c = (char) waitKey(25);

    // esc ends the show
    if(c==27) // esc
      break;
    // next frame
    else if (c==32) {
      if (frame_counter < image_list.size()) frame_counter++; else frame_counter = 0;
    }
    // s = save image
    else if (c==115){
      /*
      std::string file_quali = "./cropped_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(width)+ "_" + std::to_string(height) + "_";
      std::string template_filename =  file_quali + image_path.filename().string();
      path template_path(template_filename);
      std::cout << "saving image" << template_path << std::endl;

      Mat cropped = frame(area);
      cv::imshow("cropped image ",cropped);
      imwrite(template_path, cropped);
      cv::waitKey(0);
      */

    }
  }

  return 0;
}
