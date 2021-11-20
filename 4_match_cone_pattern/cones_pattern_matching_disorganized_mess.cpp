#include <iostream>

#include "file_operations.h"
#include "image_operations.h"
#include "histogram_class.h"
#include "opencv2/opencv.hpp"
using namespace cv;

// to manage the callbackfunctions for the opencv sliders with global variables
const int slider_max = 255;
int alpha_slider;
double alpha;
double beta;

//hsv parameters preset
int iLowH = 0;
int iHighH = 70;
int iLowS = 100;
int iHighS = 110;
int iLowV = 80;
int iHighV = 200;

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
static void tb_a_low( int, void* ){}
static void tb_a_high( int, void* ){}
static void tb_b_low( int, void* ){}
static void tb_b_high( int, void* ){}
static void tb_c_low( int, void* ){}
static void tb_c_high( int, void* ){}

// crops a rectangle from the original image
cv::Mat crop_cone_from_image(const cv::Mat &image, const int x, const int y, const int width, const int height){
  Mat ROI(image, Rect(x,y,width,height));
  Mat croppedImage;
  ROI.copyTo(croppedImage);
  return croppedImage;
}

// matches the pattern and returns a heat map
Mat match(const Mat &img, const Mat &templ, const TemplateMatchModes &match_method){

  int result_cols = img.cols - templ.cols + 1;
  int result_rows = img.rows - templ.rows + 1;

  Mat result;
  result.create( result_rows, result_cols, CV_32FC1 );
  matchTemplate( img, templ, result, match_method);
  return result;
}

void put_text(cv::Mat &image, std::string this_text, int x=10, int y=30, cv::Scalar font_color = CV_RGB(0, 0, 0)){
  cv::putText(image, //target image
          this_text, //text
          cv::Point(x,y), // position width / height //top-left position histogram.cols / 2
          cv::FONT_HERSHEY_DUPLEX,
          1.0, // fontsize
          font_color, // color 0 = black //CV_RGB(0, 185, 0), //font color
          2  // line thickness
        );
}


// converts image to HSV and filters for the specified color into a binary
cv::Mat filter_hsv_for_color(const cv::Mat &image){

  //hsv parameters preset
  int iLowH = 0;
  int iHighH = 70;
  int iLowS = 100;
  int iHighS = 255;
  int iLowV = 70;
  int iHighV = 200;

  // preset the hsv filter
  cv::Scalar lower = cv::Scalar(iLowH, iLowS, iLowV);
  cv::Scalar higher = cv::Scalar(iHighH, iHighS, iHighV);

  //cv::Mat result = image.clone();
  cv::Mat result;
  //double scale_factor = 1.0/4.0;
  //resize(image,result, cv::Size(), scale_factor, scale_factor, cv::INTER_LINEAR);

  cv::Mat imgHSV, threshold_output;
  cv::cvtColor(image, imgHSV, cv::COLOR_BGR2HSV);

  //cvtColor(frame, imgHSV, COLOR_BGR2HSV);
  cv::inRange(imgHSV, lower, higher, threshold_output);
  //imshow("Thresholded Image", img_thresh);
  return threshold_output;
}

std::vector<cv::Point> contoursConvexHull( std::vector<std::vector<cv::Point> > contours )
{
    std::vector<cv::Point> result;
    std::vector<cv::Point> pts;
    for ( size_t i = 0; i< contours.size(); i++)
        for ( size_t j = 0; j< contours[i].size(); j++)
            pts.push_back(contours[i][j]);
    cv::convexHull( pts, result );
    return result;
}

cv::Mat create_cone_template(){

  int cone_color = 0; //39;
  int background_color = 255;//71;

  int width = 100, height = 100, cone_width = 30, cone_height = 60, y_offset = 10;

  cv::Mat cone_template = cv::Mat::zeros(height,width, CV_8UC1);
  cone_template.setTo(Scalar(background_color));

  std::vector< cv::Point2i> cone_points;
  cv::Point2i ll = cv::Point2i( ( width - cone_width)/2 , cone_height + y_offset );
  cone_points.push_back(ll);
  cv::Point2i lr = cv::Point2i( ( width + cone_width)/2 , cone_height + y_offset );
  cone_points.push_back(lr);
  cv::Point2i top = cv::Point2i( ( width )/2 , 3*y_offset);
  cone_points.push_back(top);

  //std::cout << cone_points << std::endl;
  //std::cout << cone_template1.size() << std::endl;

  const cv::Point *pt4 = &(cone_points[0]);
  int num = (int)cone_points.size();

  cv::fillPoly(cone_template, &pt4, &num, 1, cv::Scalar(cone_color), 8);

  return cone_template;
}

int main(int, char**)
{
  // main window displays the original camera image and the sliders
  namedWindow("original", WINDOW_AUTOSIZE);
  createTrackbar( "threshold", "original", &iLowH, 100, tb_a_low );

  // channel, max is 2, where used in code verify that setting is permissible (i.e. < channels())
  createTrackbar( "channel", "original", &iHighH, 3, tb_a_high );

  createTrackbar( "background_high", "original", &iHighH, slider_max, tb_b_high );
  createTrackbar( "background_low", "original", &iLowS, slider_max, tb_b_low );
	createTrackbar( "cone_high", "original", &iHighS, slider_max, tb_c_high );
	createTrackbar( "cone_low", "original", &iLowV, slider_max, tb_c_low );

  /*
	createTrackbar( "b_high", "original", &iHighV, slider_max, tb_a_high );
  */

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

  // preset the hsv filter
  //Scalar lower = Scalar(iLowH, iLowS, iLowV);
  //Scalar higher = Scalar(iHighH, iHighS, iHighV);


  int x=100, y = 100;
  int width = 20, height = 60;

  int frame_counter = 0;
  int pattern_counter = 0;

  // for this solution work with grayscale
  ImreadModes image_mode = IMREAD_GRAYSCALE; //IMREAD_COLOR;//IMREAD_GRAYSCALE; //

  namedWindow("template", WINDOW_AUTOSIZE);


  // until press escape
  while(1){

    // read and display the current cone pattern
    path pattern_path = pattern_list.at(pattern_counter);
    //cone_template = imread(pattern_path, image_mode );
    cone_template = create_cone_template();
    cone_template =  cv::Scalar::all(255) - cone_template;
    imshow("template", cone_template);

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    std::cout << "loading image " << frame_counter << std::endl;
    frame = imread(image_path, image_mode );
    imshow("original", frame);

    equalizeHist(frame,frame);
    imshow("original1", frame);


    // this may cause issues when using rgb instead  of gray images
    // the histogram function currently only considers  single channel input and writes the result in the given result image no checking of channels etc
    cv::Mat histo_plot = frame.clone(), histo_data = cv::Mat();
    get_histogram(frame, 256, histo_plot, histo_data);
    imshow("histo", histo_plot);

    cv::Mat range_image = frame.clone();
    //inRange(frame, iLowV, iHighS, range_image);

    //int threshold_type = THRESH_TOZERO;//THRESH_TOZERO; THRESH_TOZERO_INV //3; // binary threshold  THRESH_TRUNC

    // dark first
    float threshold_value = iLowV; //iLowH; //(float) iLowH / 100.0;
    threshold( frame, range_image, threshold_value, -1, THRESH_TOZERO );
    threshold_value = iHighS;
    threshold( range_image, range_image, threshold_value, -1, THRESH_TOZERO_INV );

    equalizeHist(range_image,range_image);

    //cv::normalize( range_image, range_image, 0, 255, cv::NORM_MINMAX, -1, cv::Mat() );

    imshow("filter", range_image);


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
    cv::Mat match_result = match(range_image, cone_template, match_method);
    //or
    //cv::Mat match_result = match_result.clone();
    //adaptiveThreshold(range_image, match_result, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 13,0);

    imshow("match_result", match_result);

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

    imshow("result", result);

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
    // left
    else if (c==97) {
        //std::cout << "left" << std::endl;
        if ( x>0) x--;
    }
    // right
    else if (c==100) {
        //std::cout << "right" << "," << frame.size().width << "," << x << "," << width << std::endl;
        if (x < (frame.size().width - width) ) {
            x++;
        }
    }
    // up
    else if (c==120) {
      //std::cout << "up" << std::endl;
      if (y < frame.size().height - height) y++;
    }
    // down
    else if (c==119) {
        //std::cout << "down" << std::endl;
        if (y>0) y--;
    }
    // q = window width -
    else if (c==114) {
        //std::cout << "down" << std::endl;
        if (width>10) width--;
    }
    // r = window width -
    else if (c==116) {
        //std::cout << "down" << std::endl;
        if (width>10) width++;
    }
    // h = window height +
    else if (c==104) {
        //std::cout << "down" << std::endl;
        if (width>10) height--;
    }
    // n = window width -
    else if (c==110) {
        //std::cout << "down" << std::endl;
        if (width>10) height++;
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
