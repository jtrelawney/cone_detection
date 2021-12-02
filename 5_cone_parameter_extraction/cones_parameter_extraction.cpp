
#include <iostream>
#include <math.h>

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

  //while (cv::waitKey(0)!=-1);

  // main window displays the original camera image and the trackbars, as well as the cone template
  cv::namedWindow("original", cv::WINDOW_AUTOSIZE);
  cv::moveWindow("original",50,50);

  cv::namedWindow("main", cv::WINDOW_AUTOSIZE);
  cv::moveWindow("main",50,500);

  cv::waitKey(0);

	//cv::createTrackbar( "filter_low", "original", &filter_low_tracker, 255, tb_filter_low );

  cv::createTrackbar( "filter_sat", "original", &filter_sat_tracker, 255, tb_filter_sat );
  cv::createTrackbar( "tb_filter_val", "original", &filter_val_tracker, 255, tb_filter_val );

  cv::createTrackbar( "hough_threshold", "original", &hough_threshold_tracker, 1000, tb_hough_threshold );
  cv::createTrackbar( "rho_param", "original", &rho_tracker, 100, tb_rho_param );
  cv::createTrackbar( "theta_param", "original", &theta_tracker, 360, tb_theta_param );
//  cv::createTrackbar( "binarize", "original", &binarize_select, 1, button_binarize_onoff );

  // display manager with 4 split windows displays results
  display_manager window_man(4);

  // the directory needs to exist with images and the imagetype needs to match the mat type it is returned into!!!
  cv::ImreadModes image_mode = cv::IMREAD_COLOR; //IMREAD_COLOR;//IMREAD_GRAYSCALE;
  image_reader ir_images("../../cone_movies/run4", image_mode);
  //image_reader ir_cones("../../cone_templates/nice", image_mode);
  image_reader ir_cones("../../cone_templates/nice_with_extra_space", image_mode);

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
      //if (binarize_onoff == true)
      //  cone_template = binarize_image_by_color_range(cone_template, track_cone_low, track_cone_high);
    }

    // load the current frame
    frame = ir_images[frame_counter];

    // the cone template is too small resize it
    //cv::Mat resized_cone;
    //cv::resize(cone_template, resized_cone, cv::Size(640,480), cv::INTER_LINEAR);

    // during first iteration configure the window manager with 4 split images
    if (window_man.is_configured==false) {
      window_man.configure(cv::Mat(cv::Size(640,480),cone_template.type()));
      window_man.set_window("original",0);
      window_man.set_window("filter",1);
      window_man.set_window("sizing",2);
      window_man.set_window("result",3);
    }

    // convert to hsv and separate the channels to get access to the color channel
    cv::Mat hsv_image;
    cv::cvtColor(cone_template, hsv_image, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> color_channels;
    cv::split(hsv_image, color_channels);

    //sat_high val_high
    cv::ThresholdTypes ttype = cv::THRESH_TOZERO;
    int sat_low = filter_sat; //120;
    cv::threshold(color_channels[1], color_channels[1], sat_low, 0, ttype);
    int val_low = filter_val; //;//90;
    cv::threshold(color_channels[2], color_channels[2], val_low, 0, ttype);

    cv::Mat filtered_cone = color_channels[1] + color_channels[2];

    cv::Mat sizing;
    cv::GaussianBlur(filtered_cone, sizing, cv::Size(9, 9), 4.0);

    int threshold = 230;
    cv::threshold( sizing, sizing, threshold, 255, cv::THRESH_BINARY );

    cv::Mat edges;
    cv::Canny(sizing, edges, 200, 200);

    //std::vector<std::vector<cv::Point> > contours, lines;
    //std::vector<cv::Vec4i> hierarchy;

    //cv::findContours( sizing, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    //std::cout << hierarchy.size() << std::endl;
    //for (auto c:contours){
    //  std::cout << c << std::endl;
    //}
    /*
    for cnt in contours:
        cv::approxPolyDP(cnt,0.02 * cv::arcLength(cnt,true),true);
        if len(approx)==3:
            cv2.drawContours(img,[cnt],0,(0,255,0),2)
            tri = approx


    lines.resize(contours.size());
    for( size_t k = 0; k < contours.size(); k++ )
    cv::approxPolyDP(Mat(contours[k]), lines[k], 3, true);

    */




    //std::vector<cv::Vec2f> lines;
          //double  rho_param = 20;//1
          //double theta_param = CV_PI/180;
          //int hough_threshold = 100;
          //rho_param = 20;
          //theta_param = CV_PI/180;
          //hough_threshold = 100;

          // hough_threshold = minimum number of points to detect a line
          // rho_param, theta_param = accuracies

          //std::cout <<"rho_param = " << rho_param << std::endl;
    //cv::HoughLines(edges, lines, rho_param, theta_param, hough_threshold);//, 0, 0 ); // runs the actual detection

    // image to draw the lines
    cv::Mat cone_with_lines = cone_template.clone();
    //print_image_info(cone_with_lines,"cone_with_lines");

    // cone_theta : expected angle from measuring the real cone
    double cone_theta = 110;

    // iterative parameters for the find routine
    int iterations = 15;
    double step_size = 1.0;

    // find the best fitting line
    int result_rho;
    double result_theta;

    find_cone(edges, cone_theta, iterations, step_size, result_rho, result_theta);
    //std::cout << "cone theta (opencv coordinates)= " << to_deg(result_theta) << ",   rho = " << result_rho << std::endl;
    plot_cone_line(cone_with_lines, result_theta, result_rho);

    cone_theta = 70;
    find_cone(edges, cone_theta, iterations, step_size, result_rho, result_theta);
    plot_cone_line(cone_with_lines, result_theta, result_rho);

    //window_man.set_image("filtered", filtered);
    window_man.set_image("original", cone_template);
    window_man.set_image("filter", filtered_cone, true);
    window_man.set_image("sizing", edges, true);
    window_man.set_image("result", cone_with_lines, true);

    cv::Mat fucker = window_man.get_image();
    cv::imshow("main",fucker);



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


    cv::Mat display_filtered_cone = filtered_cone.clone();
    cv::resize(display_filtered_cone, display_filtered_cone, cv::Size(640,480), cv::INTER_LINEAR);
    window_man.set_image("filter", display_filtered_cone);

    cv::Mat cone_with_lines = cone_template.clone();


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
