
#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"
using namespace cv;

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
  namedWindow("original", WINDOW_AUTOSIZE);

  //createTrackbar( variable name on slider , window , &variable, slider_max, callback function);
  //createTrackbar( "threshold", "original", &iLowH, 100, tb_a_low );
  //createTrackbar( "channel", "original", &iHighH, 3, tb_a_high );
  //createTrackbar( "background_high", "original", &iHighH, 255, tb_b_high );
  //createTrackbar( "background_low", "original", &iLowS, 255, tb_b_low );
	createTrackbar( "cone_high", "original", &iHighS, 255, tb_c_high );
	createTrackbar( "cone_low", "original", &iLowV, 255, tb_c_low );

  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // read the list with cone patterns, one will be used to find the cone in the image
  std::filesystem::path cone_pattern_dir_path = "../../cone_templates";
  std::vector<path> pattern_list;
  pattern_list = read_image_list(cone_pattern_dir_path);

  // opencv global variables
  Mat frame, display_image, cone_template, pre_processed_frame;

  // index for the images in the video and index for the various cone template images
  int frame_counter = 0;
  int pattern_counter = 0;

  // define which mode the images are read from file
  // for this app we work with grayscale
  ImreadModes image_mode = IMREAD_GRAYSCALE; //IMREAD_COLOR;//IMREAD_GRAYSCALE; //

  // make this window autosize, because the templates have different sizes
  namedWindow("original", WINDOW_AUTOSIZE);
  moveWindow("original", 1000,1000);
  //namedWindow("template", WINDOW_AUTOSIZE);
  //moveWindow("original", 120,20);

  namedWindow("main", WINDOW_AUTOSIZE);

  display_manager window_man(4);
  bool window_man_configured(false);

  // until press escape
  while(1){

    // read and display the current cone pattern
    path pattern_path = pattern_list.at(pattern_counter);
    //cone_template = imread(pattern_path, image_mode );
    cone_template = create_cone_template();
    //cone_template =  cv::Scalar::all(255) - cone_template;
    //imshow("template", cone_template);

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    std::cout << "loading image " << frame_counter << std::endl;
    frame = imread(image_path, image_mode );
    //imshow("original", frame);

    // prepare the main image with cone inset and histogram
    cv::Mat main_image = frame.clone();

    cv::Rect roi_cone = cv::Rect(0,0, cone_template.cols, cone_template.rows);
    cv::Mat insetImage(main_image, roi_cone );
    cone_template.copyTo(insetImage);

    cv::Mat histo_data_orig = cv::Mat();
    get_histogram(frame, 256, main_image, histo_data_orig);
    imshow("original", main_image);

    if (window_man.is_configured==false) {
      window_man.configure(frame);
      window_man.set_window("original",0);
      window_man.set_window("filter",1);
      window_man.set_window("match",2);
      window_man.set_window("result",3);
    }

    equalizeHist(frame,frame);
    //imshow("original1", frame);


    // this may cause issues when using rgb instead  of gray images
    // the histogram function currently only considers  single channel input and writes the result in the given result image no checking of channels etc
    cv::Mat histo_plot = frame.clone(), histo_data = cv::Mat();
    get_histogram(frame, 256, histo_plot, histo_data);

    cv::Mat range_image = frame.clone();
    //inRange(frame, iLowV, iHighS, range_image);

    //int threshold_type = THRESH_TOZERO;//THRESH_TOZERO; THRESH_TOZERO_INV //3; // binary threshold  THRESH_TRUNC

    // dark first
    float threshold_value = track_cone_low;
    threshold( frame, range_image, threshold_value, -1, THRESH_TOZERO );
    threshold_value = track_cone_high;
    threshold( range_image, range_image, threshold_value, -1, THRESH_TOZERO_INV );

    equalizeHist(range_image,range_image);

    //cv::normalize( range_image, range_image, 0, 255, cv::NORM_MINMAX, -1, cv::Mat() );

    //imshow("filter", range_image);


    window_man.set_image("original", frame);
    window_man.set_image("filter", range_image);

    //window_man.set_image("original", frame);

    //inRange(frame, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), range_image);

    // cone and background color slider search
    /*
    // clone the original in grey mode
    cv::Mat hld_cone_low = frame.clone();
    //cv::Mat result_threshold( frame.rows, frame.cols, CV_8UC1 );



    //std::cout << result.type() << std::endl;
    //std::cout << result_threshold.type() << std::endl;
    //int threshold_value = 100;
    int max_binary_value = 255;
    int threshold_type = THRESH_TOZERO_INV;//THRESH_TOZERO;//3; // binary threshold  THRESH_TRUNC
    float threshold_value = iLowV;//iLowH; //(float) iLowH / 100.0;
    threshold( frame, hld_cone_low, threshold_value, max_binary_value, threshold_type );
    imshow("threshold cone low", hld_cone_low);

    cv::Mat hld_cone_high;
    threshold( frame, hld_cone_high, iHighS, max_binary_value, threshold_type );
    imshow("threshold cone high", hld_cone_high);

    cv::Mat hld_background_low;
    threshold( frame, hld_background_low, iLowS, max_binary_value, threshold_type );
    imshow("threshold background low", hld_background_low);

    cv::Mat hld_background_high;
    threshold( frame, hld_background_high, iHighH, max_binary_value, threshold_type );
    imshow("threshold background high", hld_background_high);
    */


    // run the match
    TemplateMatchModes match_method = TM_CCOEFF_NORMED; //TM_SQDIFF TM_SQDIFF_NORMED TM_CCORR TM_CCORR_NORMED TM_CCOEFF TM_CCOEFF_NORMED
    //either
    cv::Mat match_result = heatmap_from_template_match(range_image, cone_template, match_method);
    //or
    //cv::Mat match_result = match_result.clone();
    //adaptiveThreshold(range_image, match_result, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 13,0);


    cv::Mat resized_match_result;
    cv::resize(match_result, resized_match_result, Size(640,480), INTER_LINEAR);

    std::cout << "resize from = " << match_result.rows << "," << match_result.cols << " , to size = " << resized_match_result.rows << "," << resized_match_result.cols << std::endl;
    //imshow("match_result", resized_match_result);
    window_man.set_image("match", resized_match_result);

    cv::Mat result = frame.clone();
    std::vector<cv::Point> extremes = find_local_minima(match_result);

    std::cout << "drawing " << extremes.size() << " cones " << std::endl;
    for (auto which_one: extremes){
      std::cout << which_one << std::endl;
      Point ur = cv::Point(which_one.x + cone_template.size().width, which_one.y + cone_template.size().height);
      Scalar line_Color(0, 0, 255);
      int thickness = 2;
      rectangle(result, which_one, ur, line_Color, thickness);
    }

    window_man.set_image("result", result);
    cv::Mat fucker = window_man.get_image();
    imshow("main",fucker);
    //imshow("result", result);

    /*
    // find the extremes
    double mini, maxi;
    cv::Point2i minloc(0,0),maxloc(0,0);;
    cv::minMaxLoc(match_result,&mini,&maxi,&minloc,&maxloc);

    //display the find
    cv::Point2i which_one;
    switch(match_method){
      case TM_CCOEFF_NORMED:
        which_one = minloc;
        case TM_SQDIFF:
        which_one = minloc;
      default:
      which_one = maxloc;
    }

    Point ll (which_one);
    Point ur = cv::Point(which_one.x + cone_template.size().width, which_one.y + cone_template.size().height);
    Scalar line_Color(0, 0, 255);
    int thickness = 2;
    cv::Mat result = frame.clone();
    rectangle(result, ll, ur, line_Color, thickness);
    imshow("result", result);

    //std::cout << "minloc = " << minloc << " , ll = " << ll << " , ur = " << ur << std::endl;
    */

    /*
    cv::Mat small;
    double scale_factor = 1.0/1.0;
    resize(frame,small, cv::Size(), scale_factor, scale_factor, cv::INTER_LINEAR);

    // filter orange color
    cv::Mat orange_mask = filter_hsv_for_color(small);
    imshow("hsv_filter", orange_mask);

    cv::Mat keep_organge_mask = orange_mask.clone();

    // find the contours in the binary image
    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    // RETR_LIST : child / parent levels omitted, ie. the hierarchy contains all contours with no inner / outer relationship
    findContours(orange_mask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));


    //cv::RETR_TREE, RETR_LIST, RETR_CCOMP
    //cv::CHAIN_APPROX_SIMPLE,cv::RETR_EXTERNAL,

    // draw the contours

    cv::Mat result = small.clone();

    cv::Mat contour_frame = cv::Mat::zeros(keep_organge_mask.rows, keep_organge_mask.cols, CV_8UC1);//(keep_organge_mask.size(), CV_8UC1);

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

        //cv::convexHull(cv::Mat(contours[k]), hull_points[k], False);
        //cv::convexHull( contours, hull_points );

        //const std::vector<cv::Point>& contour = contours[k];
        //for (size_t k2 = 0; k2 < contour.size(); ++k2)
        //{
        //    const cv::Point& p = contour[k2];
        //    contour_frame.at<uint8_t>(p) = 255;
        //}

      int contour_count = contours.size();
      //std::cout << "contours found = " << contour_count << std::endl;



      if ( (child_contour > -1) & (parent_contour >-1) ) {
        //std::cout << "fucked up, hierarierchy child/parents set for retr_list" << std::endl;
      }

      //std::cout << "contour = " << k << "   next = " << next_contour << "   prev = " << prev_contour << "   child = " << child_contour << "   parent = " << parent_contour << std::endl;

      //
      //int count = hull_points.size();
      //std::cout << "found = " << count << std::endl;
      //if (count>2)
      //  cv::polylines( result, hull_points, true, Scalar(0,0,255), 2 );
  }

    imshow("contours", contour_frame);

    //cv::MemStorage* storage;

    //std::vector<cv::Point> ConvexHullPoints =  contoursConvexHull(contours);

    //cv::Mat matContour(threshold_output.size(), CV_8UC1);
    //cv::fillPoly(cone_template1, &cone_points, cv::Scalar(255), cv::FILLED);
    imshow("template", cone_template);

    imshow("result", result);

    TemplateMatchModes match_method = TM_CCOEFF_NORMED; //TM_SQDIFF TM_SQDIFF_NORMED TM_CCORR TM_CCORR_NORMED TM_CCOEFF TM_CCOEFF_NORMED
    Mat match_result = match(orange_mask, cone_template, match_method);
    //imshow("match_result", match_result);

    //cv::polylines( result, ConvexHullPoints, true, Scalar(0,0,255), 2 );


    //auto arround = cv::approxPolyDP(contours, sizeof(cv::Contour), storage, cv::POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);

*/

/*


    //pre_processed_frame = frame.clone();
    //histogram_class histo = histogram_class(frame);

    // fetch the requested channel, if it is 3 that means use all channels
    int fetch_channel = iHighH;
    std::string info_text = "CH = all";
    //if (fetch_channel>frame.channels()-1) fetch_channel = frame.channels()-1;
    if (fetch_channel < 3){
      pre_processed_frame = histo.get_data(frame,fetch_channel);
      cone_template = histo.get_data(cone_template,fetch_channel);
      info_text = "CH = " + std::to_string(fetch_channel) + " / " + std::to_string(frame.channels()-1);
    } else if (fetch_channel == 3) {
      pre_processed_frame = frame.clone();
    }
    //std::cout << info_text << std::endl;

    put_text(pre_processed_frame,info_text);
    imshow("template", cone_template);
    imshow("preprocessed", pre_processed_frame);

    // crazy accu
    cv::Mat channels_added;
    channels_added = histo.add_channels(frame);

    double min, max;
    minMaxLoc(channels_added, &min, &max);
    //std::cout << min << " , " << max << std::endl;

    channels_added = histo.scale_image(channels_added);
    //channels_added.convertTo(channels_added, CV_32F, 1/max, -min);

    minMaxLoc(channels_added, &min, &max);
    //std::cout << min << " , " << max << std::endl;

    imshow("image_added", channels_added);





    //Mat whatever;
    //result.convertTo(whatever, CV_8U);

    double mini, maxi;
    minMaxLoc(result, &mini, &maxi);
    std::string min_max_text = "Min/Max = " + std::to_string(mini) + " / " + std::to_string(maxi);
    put_text(result,min_max_text);

    //imshow("match", result);

    cv::Mat smaller = histo.filter_hsv_for_color(frame);
    min_max_text = "x/y = " + std::to_string(smaller.size().width) + " / " + std::to_string(smaller.size().height);
    put_text(smaller,min_max_text);
    imshow("smaller", smaller);

    //cv::Mat hsv_filter = histo.filter_hsv_for_color(smaller);

    //imshow("hsv_filter", hsv_filter);
    */
    /*

      std::vector< std::vector<cv::Point> > contours;
    	std::vector<cv::Vec4i> hierarchy;
      findContours(threshold_output, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

      cv::Mat matContour(threshold_output.size(), CV_8UC1);

        for (size_t k = 0; k < contours.size(); ++k)
        {
            const std::vector<cv::Point>& contour = contours[k];
            for (size_t k2 = 0; k2 < contour.size(); ++k2)
            {
                const cv::Point& p = contour[k2];
                matContour.at<uint8_t>(p) = 255;
            }
        }





      */



    /*
    double min, max;
    minMaxLoc(result, &min, &max);
    //std::cout << min << " , " << max << std::endl;

    //normalize( result, result, 0, 255, NORM_MINMAX, -1, Mat() );
    //imshow("normalized", result);

    minMaxLoc(result, &min, &max);
    //std::cout << min << " , " << max << std::endl;
    imshow("normalized", result);
    */



    /*

    //Mat result_threshold = result.clone();
    cv::Mat result_threshold( result.rows, result.cols, CV_8UC1 );

    std::cout << result.type() << std::endl;
    std::cout << result_threshold.type() << std::endl;
    //int threshold_value = 100;
    int max_binary_value = 255;
    int threshold_type = THRESH_TRUNC;//3; // binary threshold
    float threshold_value = (float) iLowH / 100.0;

    threshold( channels_added, result_threshold, threshold_value, max_binary_value, threshold_type );

    imshow("threshold", result_threshold);

    */

    //

    //cv::adaptiveThreshold(result, result_threshold, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY,11,2);


    //display_image = frame.clone();


    //cv::Rect area(x, y , width, height);
    //cv::rectangle( display_image, area, cv::Scalar(0, 0, 255) );

    // set the hsv filter according to the slider positions (will be set when the slider is touched)
    //lower = Scalar(iLowH, iLowS, iLowV);
    //higher = Scalar(iHighH, iHighS, iHighV);

    // apply the hsv filter and display the result
    //cvtColor(frame, imgHSV, COLOR_BGR2HSV);
    //cvtColor(frame, imgHSV, COLOR_BGR2HSV);
    //inRange(imgHSV, lower, higher, img_thresh);
    //imshow("Thresholded Image", img_thresh);



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
  }

  return 0;
}
