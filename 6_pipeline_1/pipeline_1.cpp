#include "support.h"

extern const std::vector<cv::Rect> init_view_windows;

#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"
#include <assert.h>
//#include <cassert.h>

//using namespace cv;

//using namespace cv::ml;

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
  std::filesystem::path cone_pattern_dir_path = "../../cone_templates/nice/";
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
  std::vector<WEB_PT_TYPE> orange_points;
  cv::Rect init_roi;

  // until press escape
  while(1){

    // read and display the current cone pattern
    path pattern_path = pattern_list.at(pattern_counter);
    cone_template = imread(pattern_path, image_mode );
    //cone_template = create_cone_template();
    //cv::cone_template =  cv::Scalar::all(255) - cone_template;
    cv::imshow("template", cone_template);

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    //std::cout << "loading image " << frame_counter << std::endl;
    frame = imread(image_path, image_mode );
    display_image = frame.clone();

    // at first iteration configure the window manager and other stuff
    if (window_man.is_configured==false) {
      window_man.configure(frame);
      window_man.set_window("original",0);
      window_man.set_window("filter",1);
      window_man.set_window("match",2);
      window_man.set_window("result",3);

      // region of interest where expect to find cones
      init_roi = cv::Rect(0,200,frame.cols+1,125);

      // create the web of points to check for orange
      //web.create_web(10, 10, 100, 100, 30);
      //web.create_web(10, 10, frame.cols - 10, frame.rows - 10, 10);
      web.create_web(init_roi.x, init_roi.y, init_roi.x + init_roi.width, init_roi.y + init_roi.height,10);

    }

    // convert the frame to hsv
    cv::Mat imgHSV;
    cv::cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV);

    //print_image_info(frame,"frame");
    //cv::waitKey(0);

    // find orange points in the search web
    orange_points.clear();
    WEB_PT_TYPE current = web.restart();
    //WEB_PT_TYPE current;
    //std::cout << "starting with " << current << std::endl;

    while (current != WEB_PT_TYPE()){
      //std::cout << "circling " << current << std::endl;
      cv::Vec3b color = imgHSV.at<cv::Vec3b>(current);
      //std::cout << "color " << color << std::endl;

      // mark the orange points in the web and prepare the knn data,labels
      if (is_orange(color)) {
        orange_points.push_back(current);
        //cv::circle(display_image, current, 5, cv::Scalar(0,0,255), 1);
      } //else {
        //cv::circle(display_image, current, 3, cv::Scalar(255,255,255), 1);

      //}
      current = web.get_next();
    }

    cv::TemplateMatchModes match_method = cv::TM_CCOEFF_NORMED; //TM_SQDIFF TM_SQDIFF_NORMED TM_CCORR TM_CCORR_NORMED TM_CCOEFF TM_CCOEFF_NORMED

    // all orange points in the vector
    while (orange_points.size()>0){
      // process one point from the vector
       current = orange_points.back();
       orange_points.pop_back();
       //cv::circle(display_image, current, 5, cv::Scalar(0,0,255), 1);

       //the orange point is supposed to be part of the cone, hence cut out the region of interest
       // the factor determines how much larger than the original cone template the roi should be
       // 1.0 = exactly the same size
       double roi_factor = 2.0;
       int x = current.x, y = current.y;
       int roi_width = (int) ( roi_factor * (double) cone_template.cols) , roi_height = (int) ( roi_factor * (double) cone_template.rows);
       int roi_x = (int) ( (double) x - (double) roi_width / 2.0 ) , roi_y = (int) ( (double) y - (double) roi_height / 2.0);

       // define the rect representing the roi
       cv::Rect cone_roi( roi_x, roi_y, roi_width, roi_height);
       //cv::rectangle( display_image, cone_roi, cv::Scalar(0, 0, 255),1, 8, 0 );

       // verify that the roi is inside the frame
       if (roi_x < 0) continue;
       if (roi_x + roi_width > frame.cols) continue;
       if (roi_y < 0) continue;
       if (roi_y + roi_height > frame.rows) continue;
       print_image_info(cone_template,"cone");
       //std::cout << "x= " << cone_roi.x << " w=" << cone_roi.width << " y=" << cone_roi.y << " h=" << cone_roi.height << " m cols=" << cone_template.cols << " m rows=" << cone_template.rows << std::endl;

       // crop the roi out of the frame
       cv::Mat test_roi;
       test_roi = crop_area_from_image(frame, roi_x, roi_y, roi_width, roi_height);

       // and get the heatmap
       cv::Mat match_result = heatmap_from_template_match(test_roi, cone_template, match_method);

       // when the testpatch is the same size as the template then the result is a single number, ie. the result of that particular match
       // otherwise it is a matrix and are looking for the max (depening on the match method)
       double mini,maxi;
       cv::Point minpos=current, maxpos=current;
       cv::minMaxLoc(match_result,&mini,&maxi,&minpos,&maxpos);
       //std::cout << "min = " << mini << "   max = " << maxi << "   minpos = " << minpos << "   maxpos = " << maxpos << std::endl;

       // check for reasonable match
       if (maxi>0.35){
         std::cout << "match found" << std::endl;
         cv::rectangle( display_image, cone_roi, cv::Scalar(0, 0, 255),1, 8, 0 );
         //cv::circle(display_image, minpos, 5, cv::Scalar(0,255,0), 1);
       } else continue;

       imshow("region", test_roi);
       imshow("match", match_result);
       //cv::waitKey(0);
     }

     frame_counter++;

    //draw to roi into the image
    cv::rectangle( frame, init_roi, cv::Scalar(0, 0, 255),1, 8, 0 );

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
