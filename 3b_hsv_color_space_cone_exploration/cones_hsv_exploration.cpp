
#include <iostream>

#include "support.h"
#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"

int main(int, char**)
{

  //std::cout <<"blank                    : iterates through the images" << std::endl;
  std::cout <<"press blank              : iterates through the cone templates" << std::endl;
  std::cout <<"color space              : hsv, but can select others" << std::endl;
  std::cout <<"hue_low                  : hue value low" << std::endl;
  std::cout <<"hue_high                 : hue value high" << std::endl;
  std::cout <<"sat_low                  : saturation value low" << std::endl;
  std::cout <<"sat_high                 : saturation value high" << std::endl;
  std::cout <<"value_low                : value low" << std::endl;
  std::cout <<"value_high               : value high" << std::endl;
  std::cout <<"\npress key to continue" << std::endl;

  //while (cv::waitKey(0)!=-1);

  // main window displays the original camera image and the trackbars, as well as the cone template
  cv::namedWindow("original", cv::WINDOW_AUTOSIZE);
  cv::moveWindow("original",50,50);

  cv::waitKey(0);

  cv::createTrackbar( "hue_low", "original", &hue_low, 255, tb_hue_low );
  cv::createTrackbar( "hue_high", "original", &hue_high, 255, tb_hue_high );
  cv::createTrackbar( "sat_low", "original", &sat_low, 255, tb_sat_low );
  cv::createTrackbar( "sat_high", "original", &sat_high, 255, tb_sat_high );
  cv::createTrackbar( "val_low", "original", &val_low, 255, tb_val_low );
  cv::createTrackbar( "val_high", "original", &val_high, 255, tb_val_high );

  cv::createTrackbar( "color_space", "original", &color_space, 2, tb_col_space_select );

  // display manager with 4 split windows displays results
  display_manager window_man(4);

  // the directory needs to exist with images and the imagetype needs to match the mat type it is returned into!!!
  cv::ImreadModes image_mode = cv::IMREAD_COLOR; //IMREAD_COLOR;//IMREAD_GRAYSCALE;
  //image_reader ir_cones("../../cone_templates/nice", image_mode);
  image_reader ir_cones("../../cone_movies/run5", image_mode);
  // /home/jconnor/Projects/cone_detection/cone_movies/run4

  // genereal image variables
  cv::Mat cone_template;

  // index for the images in the video and index for the various cone template images
  int cone_counter = 0;

  // until press escape
  while(1){

    cone_template = ir_cones[cone_counter];

    cv::Mat resized_cone;
    cv::resize(cone_template, resized_cone, cv::Size(640,480), cv::INTER_LINEAR);
    imshow("original",resized_cone);

    // during first iteration configure the window manager with 4 split images
    if (window_man.is_configured==false) {
      window_man.configure(resized_cone);
      window_man.set_window("hue",0);
      window_man.set_window("sat",1);
      window_man.set_window("value",2);
      window_man.set_window("result",3);
    }


    // convert to hsv and separate the channels to get access to the color channel
    cv::Mat hsv_image;
    cv::cvtColor(cone_template, hsv_image, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> color_channels;
    cv::split(hsv_image, color_channels);

    cv::Mat histo_data_orig = cv::Mat();
    cv::Mat hue_image,sat_image, val_image;

    // hue channel = color
    hue_image = color_channels[0].clone();
    cv::resize(hue_image, hue_image, cv::Size(640,480), cv::INTER_LINEAR);
    get_histogram(hue_image, 256, hue_image, histo_data_orig);
    window_man.set_image("hue", hue_image);

    // sat channel = vibrancy
    sat_image = color_channels[1].clone();
    cv::resize(sat_image, sat_image, cv::Size(640,480), cv::INTER_LINEAR);
    get_histogram(sat_image, 256, sat_image, histo_data_orig);
    window_man.set_image("sat", sat_image);

    // val channel = brightness
    val_image = color_channels[2].clone();
    cv::resize(val_image, val_image, cv::Size(640,480), cv::INTER_LINEAR);
    get_histogram(val_image, 256, val_image, histo_data_orig);
    window_man.set_image("value", val_image);


    //threshold arround the cone colors (dominant on the handpicked cone image)
    cv::Mat hue_filtered, sat_filtered, val_filtered;
    int  max_value = 0; // for thresh_binary and thresh_binary_inv only

    // cut everything above max to 0
    cv::ThresholdTypes ttype = cv::THRESH_TOZERO_INV; //THRESH_TOZERO; //cv::THRESH_BINARY; THRESH_TOZERO_INV
    cv::threshold(color_channels[0], hue_filtered, hue_high, 0, ttype );

    // cut everything below to 0
    ttype = cv::THRESH_TOZERO;
    cv::threshold(hue_filtered, hue_filtered, hue_low, 0, ttype );

    // binarize everthing that is not supposed to be 0 to 255
    //ttype = cv::THRESH_BINARY;
    //cv::threshold(hue_filtered, hue_filtered, hue_low, 255, ttype );

    //sat_high val_high
    ttype = cv::THRESH_TOZERO;
    cv::threshold(color_channels[1], sat_filtered, sat_low, 0, ttype);
    cv::threshold(color_channels[2], val_filtered, val_low, 0, ttype);

    // stitch the image together and convert to standard
    cv::Mat result;
    std::vector<cv::Mat> filtered_channels = { hue_filtered, sat_filtered, val_filtered};
    cv::merge(filtered_channels,result);
    cv::cvtColor(result, result, cv::COLOR_HSV2BGR);

    cv::Mat shortcut = color_channels[1] + color_channels[2];
    cv::Rect roi_shortcut = cv::Rect(0,200, shortcut.cols, 150);
    cv::Mat shortcut_focus(shortcut, roi_shortcut );
    cv::imshow("shortcut",shortcut_focus);



    //cv::Mat result = cone_template.clone();
    cv::resize(result, result, cv::Size(640,480), cv::INTER_LINEAR);
    window_man.set_image("result", result);


    /*
    //cv::Mat color_image(color_channels[2]);

    // add the histogram and display
    cv::Mat main_image = frame.clone();
    cv::Mat histo_data_orig = cv::Mat();
    get_histogram(color_channels[2], 256, main_image, histo_data_orig);

    // convert to hsv and get stats for color channel
    cv::cvtColor(cone_template, hsv_image, cv::COLOR_BGR2HSV);
    cv::split(hsv_image, color_channels);
    get_histogram(color_channels[2], 256, main_image, histo_data_orig);

    //cv::Mat kernel = (cv::Mat_<float>(3,1) << 10, 20, 30 );
    double mu,stddev;
    gaussian g(histo_data_orig);
    g.get_params(mu,stddev);

    //threshold arround the cone colors (dominant on the handpicked cone image)
    cv::threshold(color_channels[2], color_channels[2], mu-20, mu+20, cv::THRESH_BINARY);

    // stitch the image together and convert to standard
    std::vector<cv::Mat> channels = {color_channels[0],color_channels[1],color_channels[2]};
    cv::Mat binarized_image;
    cv::merge(channels,binarized_image);
    cv::cvtColor(binarized_image, binarized_image, cv::COLOR_HSV2BGR);

    //std::cout << mu << "," << stddev << std::endl;


    //Equalize the histogram of the Y channel
    //cv::equalizeHist(color_channels[2], color_channels[2]);
    //cv::equalizeHist(color_channels[1], color_channels[1]);
    //cv::equalizeHist(color_channels[2], color_channels[2]);

    // merge back and convert to bgr
    //cv::merge(color_channels, result_image);
    //cv::cvtColor(result_image, result_image, cv::COLOR_YCrCb2BGR);
    //cv::cvtColor(result_image, result_image, cv::COLOR_HSV2BGR);


    // prepare the original image with cone inset and histogram
    cv::Rect roi_cone = cv::Rect(0,0, cone_template.cols, cone_template.rows);
    cv::Mat insetImage(main_image, roi_cone );
    cone_template.copyTo(insetImage);
    cv::imshow("original", main_image);



    //cv::Mat equalized_cone = cv::Mat(), filler =  cv::Mat();
    //get_histogram1(cone_template, 256, equalized_cone, filler);

    //cv::Rect roi_other_cone = cv::Rect(cone_template.cols, 0, cone_template.cols, cone_template.rows);
    //cv::Mat other_insetImage(main_image, roi_other_cone );
    //equalized_cone.copyTo(other_insetImage);






    // the result mat is smaller -> bring back to windowsize (for display only)
    cv::Mat resized_cone;
    cv::resize(cone_template, resized_cone, cv::Size(640,480), cv::INTER_LINEAR);
    window_man.set_image("original", resized_cone);


    window_man.set_image("filter", binarized_image);

    //cv::resize(equalized_cone, equalized_cone, cv::Size(640,480), cv::INTER_LINEAR);
    cv::Mat result = cone_template.clone();
    //window_man.set_image("result", equalized_cone);


    cv::Mat filtered_cone = cone_template.clone();
    cv::Canny(binarized_image,filtered_cone,0,100);
    window_man.set_image("match", filtered_cone);
    //cv::Mat kernel = cv::Mat::zeros(3,3, CV_32F);
    //kernel << [1,0,1,1,0,1,1,0,1];

    //cv::Mat kernel = (cv::Mat_<double>(3,3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
    //cv::Mat kernel = (cv::Mat_<double>(3,3) << -1, 0, -1, -1, 5, -1, 0, -1, 0);

    //cv::Mat kernel = (cv::Mat_<double>(4,5) << -1, -1, 0, 1, 1,   -1, 0, 1, 1, 1 ,  -1, 0, 1, 1, 1,   0, 1, 1, 1, 1);
    //cv::Mat kernel = (cv::Mat_<double>(4,5) << -1, -1, 0, 1,   -1, -1, 0, 1,   -1, 0, 1, 1,   -1, 0, 1, 1 ,   0, 1, 1, 1);

    // works ok
    //cv::Mat kernel = (cv::Mat_<double>(6,4) << -1, -1, 0, 1,   -1, -1, 0, 1,   -1, 0, 1, 1,   -1, 0, 1, 1 ,   0, 1, 1, 1,   0, 1, 1, 1);

    //cv::Mat kernel = (cv::Mat_<double>(6,4) <<  -1, -1, 0, 1,
      //                                         -1, -1, 0, 1,
        //                                       -1, 0, 1, 1,
          //                                     -1, 0, 1, 1 ,
            //                                    0, 1, 1, 1,
              //                                  0, 1, 1, 1);
            */
/*
    //cv::Mat kernel = (cv::Mat_<double>(3,4) << -1, 0, 0, 1,  -2, 0, 0, 2,   -1, 0, 0, 1);

    // a hint of robustness
    //cv::Mat kernel = (cv::Mat_<double>(2,4) << 0, -1, 0, 1    -1, 0, 0, 1);
    cv::Mat kernel = (cv::Mat_<double>(2,4) << 0, 2, 0, -2,    2, 0, -2,0 );

    cv::filter2D(cone_template, filtered_cone,-1,kernel);
    //cv::minMax

    cv::threshold( filtered_cone, filtered_cone, filter_low, 0, cv::THRESH_TOZERO_INV );
    std::cout << kernel << std::endl;

    cv::Mat display_filtered_cone = filtered_cone.clone();
    cv::resize(display_filtered_cone, display_filtered_cone, cv::Size(640,480), cv::INTER_LINEAR);
    window_man.set_image("filter", display_filtered_cone);

    cv::Mat cone_with_lines = cone_template.clone();

    std::vector<cv::Vec2f> lines; // will hold the results of the detection
    //double  rho_param = 20;//1
    //double theta_param = CV_PI/180;
    //int hough_threshold = 100;
    //rho_param = 20;
    //theta_param = CV_PI/180;
    //hough_threshold = 100;

    // hough_threshold = minimum number of points to detect a line
    // rho_param, theta_param = accuracies

    //std::cout <<"rho_param = " << rho_param << std::endl;
    cv::HoughLines(filtered_cone, lines, rho_param, theta_param, hough_threshold, 0, 0 ); // runs the actual detection

    // Draw the lines
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        if ( (rho>20) && (rho<22) ) {
          cv::line( cone_with_lines, pt1, pt2, cv::Scalar(0,0,255), 1, cv::LINE_AA);
          //std::cout << "rho = " << rho << "   theta = " << theta << std::endl;
        }
    }

    cv::Mat display_cone_with_lines = cone_with_lines.clone();
    cv::resize(display_cone_with_lines, display_cone_with_lines, cv::Size(640,480), cv::INTER_LINEAR);
    window_man.set_image("match", display_cone_with_lines);

    cv::Mat result = resized_cone.clone();
*/


    /*
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
    */
    // display the result window

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
      if ( cone_counter < ir_cones.get_image_count()-1 ) cone_counter++; else cone_counter = 0;
    }
    else if (c==116) { // press t
      //std::cout << "press t" << std::endl;
      //if ( template_counter < .get_image_count()-1 ) template_counter++; else template_counter = 0;
    }
  }

  return 0;
}
