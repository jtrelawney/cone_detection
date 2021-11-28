
#include <iostream>

#include "support.h"
#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"

int main(int, char**)
{

  std::cout <<"blank                    : iterates through the images" << std::endl;
  std::cout <<"t                        : iterates through the cone templates" << std::endl;
  std::cout <<"cone low and cone high   : the color thresholds for the cones" << std::endl;
  std::cout <<"equalize                 : apply image equalizion - on / off" << std::endl;
  std::cout <<"template                 : use a manual template or the cropped cone image as template - on / off" << std::endl;
  std::cout <<"binarize                 : transform grayimage to binary image using the thresholds - on / off" << std::endl;
  std::cout <<"\npress key to continue" << std::endl;

  while (cv::waitKey(0)!=-1);

  // main window displays the original camera image and the trackbars, as well as the cone template
  cv::namedWindow("original", cv::WINDOW_AUTOSIZE);
  cv::moveWindow("original",50,50);

  cv::namedWindow("main", cv::WINDOW_AUTOSIZE);
  cv::moveWindow("main",50,500);

  cv::waitKey(0);

	cv::createTrackbar( "cone_low", "original", &SatLow, 255, tb_c_low );
  cv::createTrackbar( "cone_high", "original", &SatHigh, 255, tb_c_high );
  cv::createTrackbar( "equalize", "original", &Equalize, 1, tb_c_equal );
  cv::createTrackbar( "template", "original", &template_select, 1, button_template_onoff );
  cv::createTrackbar( "binarize", "original", &binarize_select, 1, button_binarize_onoff );

  // display manager with 4 split windows displays results
  display_manager window_man(4);

  // the directory needs to exist with images and the imagetype needs to match the mat type it is returned into!!!
  cv::ImreadModes image_mode = cv::IMREAD_GRAYSCALE; //IMREAD_COLOR;//IMREAD_GRAYSCALE;
  image_reader ir_images("../../cone_movies/run4", image_mode);
  image_reader ir_cones("../../cone_templates/nice", image_mode);

  // genereal image variables
  cv::Mat frame, cone_template;

  // index for the images in the video and index for the various cone template images
  int frame_counter = 0;
  int template_counter = 0;

  // until press escape
  while(1){

    //pattern_counter=1;
    if (template_onoff == true) {
      cone_template = create_cone_template(true);
    } else {
      cone_template = ir_cones[template_counter];
      // if template cut from original is used: convert to min max
      if (binarize_onoff == true)
        cone_template = binarize_image_by_color_range(cone_template, track_cone_low, track_cone_high);
    }

    // load the current frame
    frame = ir_images[frame_counter];

    // prepare the original image with cone inset and histogram
    cv::Mat main_image = frame.clone();
    cv::Rect roi_cone = cv::Rect(0,0, cone_template.cols, cone_template.rows);
    cv::Mat insetImage(main_image, roi_cone );
    cone_template.copyTo(insetImage);

    // add the histogram and display
    cv::Mat histo_data_orig = cv::Mat();
    get_histogram(frame, 256, main_image, histo_data_orig);
    cv::imshow("original", main_image);

    // during first iteration configure the window manager with 4 split images
    if (window_man.is_configured==false) {
      window_man.configure(frame);
      window_man.set_window("original",0);
      window_man.set_window("filter",1);
      window_man.set_window("match",2);
      window_man.set_window("result",3);
    }

    // preprocess the frame
    cv::Mat preprocessed = frame.clone(), histo_data = cv::Mat();
    if (equalize_y_n==true) cv::equalizeHist(preprocessed,preprocessed);

    // this may cause issues when using rgb instead  of gray images
    // the histogram function currently only considers  single channel input and writes the result in the given result image no checking of channels etc
    //get_histogram(preprocessed, 256, preprocessed, histo_data);

    window_man.set_image("original", preprocessed);


    // apply thresholds to create a binary image with values in cone range as white and the rest black
    cv::Mat range_image = preprocessed.clone();
    if (binarize_onoff == true)
      range_image = binarize_image_by_color_range(preprocessed, track_cone_low, track_cone_high);

    /*
    cv::Mat range_image1 = preprocessed.clone();
    //int threshold_type = THRESH_TOZERO;//THRESH_TOZERO; THRESH_TOZERO_INV //3; // binary threshold  THRESH_TRUNC

    // below threshold to 0
    float threshold_value = track_cone_low;
    cv::threshold( preprocessed, range_image, threshold_value, 0, cv::THRESH_TOZERO );
    // above threshold to 0
    threshold_value = track_cone_high;
    cv::threshold( range_image, range_image, threshold_value, 0, cv::THRESH_TOZERO_INV );
    //remainder to 1;
    threshold_value = track_cone_low;
    cv::threshold( range_image, range_image, threshold_value, 255, cv::THRESH_BINARY );
    */
    window_man.set_image("filter", range_image);

    // run the match
    cv::TemplateMatchModes match_method = cv::TM_CCOEFF_NORMED; //TM_SQDIFF TM_SQDIFF_NORMED TM_CCORR TM_CCORR_NORMED TM_CCOEFF TM_CCOEFF_NORMED
    bool adaptive_y_no = false;
    cv::Mat match_result;
    if (adaptive_y_no == true){
      cv::adaptiveThreshold(range_image, match_result, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 13,0);
    } else {
      match_result = heatmap_from_template_match(range_image, cone_template, match_method);
    }

    // find the extrema
    cv::Mat result = frame.clone();
    // find 4 minima
    //std::vector<cv::Point> extremes = find_local_minima(match_result);

    std::vector<cv::Point2i> extrema_list;
    int extrema_count = find_local_extrema(match_result, cone_template, extrema_list);

    /*
    // manual extreme
    double mini, maxi;
    cv::Point2i minloc(0,0),maxloc(0,0);
    cv::minMaxLoc(match_result,&mini,&maxi,&minloc,&maxloc);
    std::cout << "mini = " << mini << " , maxi = " << maxi << " , minloc = " << minloc << " , maxloc = " << maxloc << std::endl;
    std::vector<cv::Point> extremes;
    extremes.push_back(maxloc);
    */

    //std::cout << "drawing " << extrema_list.size() << " cones " << std::endl;
    for (auto which_one: extrema_list){

      cv::Scalar result_marker(0, 0, 255), cone_marker(0, 0, 255);
      int thickness = 2, radius = 10;

      // mark the extreme in the result image
      cv::circle(match_result, which_one, radius, cone_marker, thickness);

      // mark the extreme in the frame
      cv::Point cone_center = cv::Point(which_one.x + cone_template.size().width/2, which_one.y + cone_template.size().height/2);
      cv::circle(result, cone_center, radius, 255, thickness);

      // mark the roi in the frame
      cv::Point corner1 = cv::Point(cone_center.x + cone_template.size().width/2, cone_center.y + cone_template.size().height/2);
      cv::Point corner2 = cv::Point(cone_center.x - cone_template.size().width/2, cone_center.y - cone_template.size().height/2);
      cv::rectangle(result, corner1, corner2, cone_marker, thickness);
    }

    // the result mat is smaller -> bring back to windowsize (for display only)
    cv::Mat resized_match_result;
    cv::resize(match_result, resized_match_result, cv::Size(640,480), cv::INTER_LINEAR);
    window_man.set_image("match", resized_match_result);
    //std::cout << "resize from = " << match_result.rows << "," << match_result.cols << " , to size = " << resized_match_result.rows << "," << resized_match_result.cols << std::endl;

    // display the result window
    window_man.set_image("result", result);
    cv::Mat fucker = window_man.get_image();
    cv::imshow("main",fucker);

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
      if ( frame_counter < ir_images.get_image_count()-1 ) frame_counter++; else frame_counter = 0;
    }
    else if (c==116) {
      //std::cout << "press t" << std::endl;
      if ( template_counter < ir_cones.get_image_count()-1 ) template_counter++; else template_counter = 0;
    }
  }

  return 0;
}
