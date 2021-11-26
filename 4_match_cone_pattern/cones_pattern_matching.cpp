
#include <iostream>

#include "support.h"
#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"

int main(int, char**)
{
  // main window displays the original camera image and the trackbars, as well as the cone template
  cv::namedWindow("original", cv::WINDOW_AUTOSIZE);
  cv::moveWindow("original",50,50);

  cv::namedWindow("main", cv::WINDOW_AUTOSIZE);
  cv::moveWindow("main",50,500);

	cv::createTrackbar( "cone_low", "original", &SatLow, 255, tb_c_low );
  cv::createTrackbar( "cone_high", "original", &SatHigh, 255, tb_c_high );
  cv::createTrackbar( "equalize", "original", &Equalize, 1, tb_c_equal );

  // display manager with 4 split windows displays results
  display_manager window_man(4);

  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // read the list with cone patterns, one will be used to find the cone in the image
  std::filesystem::path cone_pattern_dir_path = "../../cone_templates";
  std::vector<path> pattern_list;
  pattern_list = read_image_list(cone_pattern_dir_path);


  // define which image mode to work with   gray / color
  cv::ImreadModes image_mode = cv::IMREAD_GRAYSCALE; //IMREAD_COLOR;//IMREAD_GRAYSCALE; //

  // genereal image variables
  cv::Mat frame, display_image, cone_template;

  // index for the images in the video and index for the various cone template images
  int frame_counter = 0;
  int pattern_counter = 0;

  // until press escape
  while(1){

    // read and display the current cone pattern
    path pattern_path = pattern_list.at(pattern_counter);
    //cone_template = imread(pattern_path, image_mode );
    cone_template = create_cone_template(true);
    //cone_template =  cv::Scalar::all(255) - cone_template;

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    std::cout << "loading image " << frame_counter << std::endl;
    frame = cv::imread(image_path, image_mode );

    // prepare the oritinal image with cone inset
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


    cv::Mat range_image = preprocessed.clone();
    // apply thresholds to create a binary image with values in cone range as white and the rest black

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

    // the result mat is smaller -> bring back to windowsize
    cv::Mat resized_match_result;
    cv::resize(match_result, resized_match_result, cv::Size(640,480), cv::INTER_LINEAR);
    window_man.set_image("match", resized_match_result);
    //std::cout << "resize from = " << match_result.rows << "," << match_result.cols << " , to size = " << resized_match_result.rows << "," << resized_match_result.cols << std::endl;

    //
    cv::Mat result = frame.clone();
    // find 4 minima
    //std::vector<cv::Point> extremes = find_local_minima(match_result);

    // manual extreme
    double mini, maxi;
    cv::Point2i minloc(0,0),maxloc(0,0);
    cv::minMaxLoc(match_result,&mini,&maxi,&minloc,&maxloc);
    std::cout << "mini = " << mini << " , maxi = " << maxi << " , minloc = " << minloc << " , maxloc = " << maxloc << std::endl;
    std::vector<cv::Point> extremes;
    extremes.push_back(maxloc);

    std::cout << "drawing " << extremes.size() << " cones " << std::endl;
    for (auto which_one: extremes){

      cv::Scalar result_marker(0, 0, 255), cone_marker(0, 0, 255);
      int thickness = 2, radius = 10;

      // position of the extreme in the result mat (which is of reduced size because if the filter)
      std::cout << which_one << std::endl;
      cv::Point corner1 = which_one;
      cv::Point corner2 = cv::Point(which_one.x + cone_template.size().width, which_one.y + cone_template.size().height);
      // mark the result
      cv::rectangle(result, corner1, corner2, result_marker, thickness);
      cv::circle(result, corner1, radius, result_marker, thickness);

      // position of the extreme in the original frame - should be the center of the cone template
      cv::Point template_position = cv::Point(which_one.x + cone_template.size().width/2, which_one.y + cone_template.size().height/2);
      corner1 = cv::Point(template_position.x - cone_template.size().width/4, template_position.y - cone_template.size().height/4);
      corner2 = cv::Point(template_position.x + cone_template.size().width/4, template_position.y + cone_template.size().height/4);
      // mark the cone
      cv::rectangle(result, corner1, corner2, cone_marker, thickness);
      cv::circle(result, corner1, radius, cone_marker, thickness);
    }

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
      if (frame_counter < image_list.size()) frame_counter++; else frame_counter = 0;
    }
  }

  return 0;
}
