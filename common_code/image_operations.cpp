#include "image_operations.h"
#include <assert.h>
#include <cassert>

// apply thresholds to create a binary image with values in cone range as white and the rest black
cv::Mat binarize_image_by_color_range(const cv::Mat &image, double color_low, double color_high){
  //int threshold_type = THRESH_TOZERO;//THRESH_TOZERO; THRESH_TOZERO_INV //3; // binary threshold  THRESH_TRUNC

  cv::Mat binarized_image = image.clone();

  // below threshold to 0
  cv::threshold( image, binarized_image, color_low, 0, cv::THRESH_TOZERO );
  // above threshold to 0
  cv::threshold( binarized_image, binarized_image, color_high, 0, cv::THRESH_TOZERO_INV );
  //remainder to 1;
  cv::threshold( binarized_image, binarized_image, color_low, 255, cv::THRESH_BINARY );

  return binarized_image;
}

// finds extremes in the results image created by the template match
// result_image (type = CV_32FC1 ): result image from the template match, find the extrema in this data, which is a mat of predescribed type, as required by opencv:match()
// template: the template image, we need the size of the template to get ot the ROI
// extrema_list: the results will be pushed here, since the opencv match function returns the upper left corner of the template this is corrected to the actual coordinates of the extrema
// min_or_max: bool with false = min and true = max
// max_count: number of extremes to find before stopping the search
// threshold: the local min or max are recoginized  only if they satisfy the threshold
// roi_factor: the size of the patch which will be blacked out arround a extrema, this avoids finding several extrema in the same place for a particulare good template match
int find_local_extrema(cv::Mat &result_image, const cv::Mat &template_image, std::vector<cv::Point2i> &extrema_list, int max_count, bool min_or_max, double roi_factor, double threshold){

  // result reset
  int result_count = 0;
  extrema_list.clear();

  // extrema and their locs
  double mini, maxi;
  cv::Point2i minloc(0,0),maxloc(0,0);
  cv::minMaxLoc(result_image,&mini,&maxi,&minloc,&maxloc);

  //std::cout << "mini = " << mini << " , maxi = " << maxi << " , minloc = " << minloc << " , maxloc = " << maxloc << std::endl;

  if (min_or_max==true) extrema_list.push_back(maxloc);
  else extrema_list.push_back(minloc);
  result_count++;

  return result_count;
}

// finds extremes in the results images such as for template matching
std::vector<cv::Point> find_local_minima(const cv::Mat &image){

  std::vector<cv::Point> result;

  //cv::Mat work_image = image.clone();
  cv::Mat work_image = cv::Mat(); //image.rows, image.cols, CV_8U);
  // normalize(input, output, minvalue, maxvalue, norm, if <0 output same type as input, mask)
  cv::normalize( image, work_image, 0, 255, cv::NORM_MINMAX, -1, cv::Mat() );
  work_image.convertTo(work_image, CV_8U);

  int max_count = 4;
  double mini, maxi;

  bool done = false;
  while (!done){

    // find min and max
    cv::Point2i minloc(0,0),maxloc(0,0);
    cv::minMaxLoc(work_image,&mini,&maxi,&minloc,&maxloc);
    std::cout << "mini = " << mini << " , maxi = " << maxi << " , minloc = " << minloc << " , maxloc = " << maxloc << std::endl;

    // chose the extreme, either the min or the max
    bool min_not_max = true;
    cv::Point2i which_one = minloc;
    if (min_not_max==false) which_one = maxloc;

    // for as many matches we are looking for: save the result
    if (max_count>0) {
      result.push_back(which_one);
      max_count--;

      //  now fill in the area with the oposite extreme values, if working with max - fill in min and vice versa
      double fill_value = maxi;
      if (min_not_max==false) fill_value = mini;

      cv::Rect fill_area(which_one.x -10,which_one.y -10, which_one.x + 10, which_one.y + 10);
      cv::Scalar loDiff = 50;
      cv::Scalar upDiff = 50;

      int connectivity = 4 + (255 << 8) + cv::FLOODFILL_FIXED_RANGE;

      //cv::Rect roi(1,1, work_image.cols-2, work_image.rows-2);
      //cv::Mat work_image_roi = image(roi);

      // create he mask with requred size
      cv::Mat mask = work_image.clone();
      cv::copyMakeBorder(mask, mask, 1, 1, 1, 1, cv::BORDER_REPLICATE);
      std::cout << work_image.size() << "," << work_image.type() << "," << mask.size() << ","  << mask.type() << std::endl;

      //imshow("workimage", work_image);

      int area = cv::floodFill(work_image, which_one, fill_value, NULL, loDiff, upDiff, connectivity);
      std::cout << "area = " << area << std::endl;
      //int area = cv::floodFill(work_image, mask, which_one, fill_value, NULL, loDiff, upDiff, 4);
      //imshow("flooded", work_image);

      //imshow("mask", mask);

      //while(cv::waitKey(10) != 32);
      std::cout << "next extreme " << max_count << std::endl;
    } else done = true;

    std::cout << "which_one = " << which_one << " , done = " << done << std::endl;

  }

  //std::cout << "bye bye with " << result << std::endl;

  return result;

}


// matches the pattern and returns a heat map
cv::Mat heatmap_from_template_match( const cv::Mat &img, const cv::Mat &templ, const cv::TemplateMatchModes &match_method){

  // prepare the result image which is 1 pixel larger than the original reduced by the template and it is a C1 Float type
  cv::Mat result;
  int result_cols = img.cols - templ.cols + 1;
  int result_rows = img.rows - templ.rows + 1;
  result.create( result_rows, result_cols, CV_32FC1 );

  // match into the result
  cv::matchTemplate( img, templ, result, match_method);
  return result;
}


std::string get_image_type_str(int type) {
  std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

void print_image_info(const cv::Mat &image, std::string mat_text=""){
  std::string start_text = "image info : rows / cols = ";
  if (mat_text.size() > 0){
    start_text = "image info " + mat_text + " : rows / cols = ";
  }
  std::cout << start_text << image.rows << "," << image.cols << "   channels = " << image.channels() << "   type = " << get_image_type_str(image.type()) << std::endl;
}

// crops a rectangle from the original image
cv::Mat crop_area_from_image(const cv::Mat &image, const int x, const int y, const int width, const int height){
  cv::Mat ROI(image, cv::Rect(x,y,width,height));
  cv::Mat croppedImage;
  ROI.copyTo(croppedImage);
  return croppedImage;
}

// plots the data into the given result_image
// the result image dimensions are used to fit the histogram data into the entire image
cv::Mat plot_histo(const cv::Mat &histo_data, cv::Mat &result_image){

  // debug
  //std::cout << "histogram image size (r/c, channels) = " << result_image.rows << " , " << result_image.cols << " , " << result_image.channels() << std::endl;

  // normalize the histogram data to make it fit into the image height
  int result_image_height = result_image.rows;
  cv::normalize( histo_data, histo_data, 0, result_image_height, cv::NORM_MINMAX, -1, cv::Mat() );

  // find max value
  //double minVal, maxVal;
  //cv::minMaxLoc(histo_data, &minVal, &maxVal);
  //int hist_h = maxVal;//histo_data.max();
  //std::cout << "histo size = " << histo_data.size() << " ,  max = " << hist_w << std::endl;

  // get the bucket count from the histo data, it is its height, that is what is filled in by opencv calcHist
  int bucket_count = histo_data.size().height;
  // now the bucket count is spread over the width of the result image, ie. how many pixels does each bucket need to cover
  float bucket_2_width_factor = (double) result_image.cols / (double) bucket_count;

  // iterate through the buckets
  for( int i = 1; i < bucket_count; i++ )
  {
      // for neighbouring buckets create starting and endpoints (spread the buckets over the image width)
      int x1 = cvRound ( bucket_2_width_factor*(i-1) );
      int y1 = result_image_height - cvRound(histo_data.at<float>(i-1));
      cv::Point starting_point = cv::Point( x1, y1 );

      int x2 = cvRound ( bucket_2_width_factor*(i) );
      int y2 = result_image_height - cvRound(histo_data.at<float>(i));
      cv::Point ending_point = cv::Point( x2, y2 );

      cv::Scalar plot_color = cv::Scalar( 255, 0, 0 );

      cv::line( result_image, starting_point, ending_point, plot_color, 2, 8, 0  );

      /*
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ),
            Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
            Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ),
            Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
            Scalar( 0, 0, 255), 2, 8, 0  );
      */
  }

  return result_image;
}

#include <math.h>

gaussian::gaussian(const cv::Mat &data){

  if (data.channels() != 1) {
    std::cout << "data channels() != 1" << std::endl;
  }

  cv::Scalar m, stdv;
  cv::meanStdDev(data,m,stdv);
  this->mu = m[0];
  this->stddev = stdv[0];

  return;

  std::cout << "data entries  = " << data.rows << "   mu = " << this->mu << "   stddev = " << this->stddev << std::endl;

  this->mu = 0.0;
  for (int i=0; i<data.rows; i++){
    float d = data.at<float>(i,0);
    this->mu += d;
    //std::cout << "d " << i << " - " << d << std::endl;
  }

  this->mu = this->mu / data.rows;


  cv::Mat help(data);
  help = help - this->mu;
  std::cout << help << std::endl;
  //cv::Mat transe;
  //transpose(help,transe);
  cv::Mat square;// = help * transe;
  multiply(help, help, square);
  //std::cout << square << std::endl;

  //std::cout << cv::sum(square) << std::endl;

  float s = cv::sum(square)[0];

  this->stddev = sqrt(s/data.rows);
  //std::cout << "data entries  = " << data.rows << "   mu = " << this->mu << "   stddev = " << this->stddev << std::endl;
}
gaussian::~gaussian(){}
void gaussian::get_params(double &mu,double &stddev){ mu = this->mu; stddev = this->stddev; };
gaussian::gaussian(){}


bool get_histogram1(const cv::Mat &image, const int &bucket_count, cv::Mat &result_image, cv::Mat &result_data ){

  //result_image = cv::Mat::zeros(image.rows,image.cols,image.type());
  //return true;

  //result_image = image.clone();

  cv::Mat hsv_image;

  //cv::cvtColor(image, result_image, cv::COLOR_BGR2YCrCb);

  // convert to hsv and separate the channels to get access to the color channel
  cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);
  std::vector<cv::Mat> color_channels;
  cv::split(hsv_image, color_channels);

  cv::Mat color_image(image.rows,image.cols,CV_8UC1);
  cv::Mat r;
  get_histogram(color_image, 255, result_image, result_data );
  return true;


  //Equalize the histogram of the Y channel
  cv::equalizeHist(color_channels[2], color_channels[2]);
  //cv::equalizeHist(color_channels[1], color_channels[1]);
  //cv::equalizeHist(color_channels[2], color_channels[2]);

  // merge back and convert to bgr
  cv::merge(color_channels, result_image);
  //cv::cvtColor(result_image, result_image, cv::COLOR_YCrCb2BGR);
  cv::cvtColor(result_image, result_image, cv::COLOR_HSV2BGR);

  //result_image = hist_equalized_image.clone();

  return true;
}

// calculates and plots the histogram for the given image
// currently the function assumes 256 input values and only accepts single channel input
// the result data can be empty, the result_image must be sized because the data is fit into the full size of the result image
bool get_histogram(const cv::Mat &image, const int &bucket_count, cv::Mat &result_image, cv::Mat &result_data ){

  // ensure one channel only
  if (image.channels() != 1){
    std::cout << "get_histogram() : image has " << image.channels() << " channels, required are 1." << std::endl;
    return false;
  }

  // calculate the histogram data
  int channels[] = { 0 };
  // the number of target buckets
  int histSize[] = { bucket_count };
  // adjust here for the number of different inputs to count
  float range[] = { 0, (float) 256 };
  const float* ranges[] = { range };

  cv::calcHist(&image, 1, channels, cv::Mat(), result_data, 1, histSize, ranges, true, false);

  // debug
  //std::cout << "histogram data size (r/c, channels) = " << result_data.rows << " , " << result_data.cols << " , " << result_data.channels() << std::endl;

  // debug: print the histogram
  //std::cout << "histogram data = " << histogram.type() << std::endl;
  //  for (int i=0; i<histogram.size().height; i++){
  //  unsigned int value = histogram.at<double>(i);
  //    std::cout << "i = " << i << "     v = " << value << std::endl;
  //  }

  // plot the data into the image
  cv::Mat histo_plot = plot_histo(result_data,result_image);
  result_image = histo_plot;

  return true;
}

// choose reverse background as true, if the preprocessing arrives at a dark background with a bright cone candiate
// this sets the background for the cone template to dark too
cv::Mat create_cone_template(bool reverse_background){

  int cone_color = 0, background_color = 255;
  if (reverse_background) {
    cone_color = 255, background_color = 0;
  }

  int width = 100, height = 100, cone_width = 30, cone_height = 60, y_offset = 10;

  cv::Mat cone_template = cv::Mat::zeros(height,width, CV_8UC1);
  cone_template.setTo(cv::Scalar(background_color));

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
cv::Mat filter_hsv_for_color(const cv::Mat &image, cv::Scalar lower_bound, cv::Scalar upper_bound){

  // //hsv parameters preset
  // int iLowH = 0;
  // int iHighH = 70;
  // int iLowS = 100;
  // int iHighS = 255;
  // int iLowV = 70;
  // int iHighV = 200;

  // preset the hsv filter
  //cv::Scalar lower = cv::Scalar(iLowH, iLowS, iLowV);
  //cv::Scalar higher = cv::Scalar(iHighH, iHighS, iHighV);

  //cv::Mat result = image.clone();
  cv::Mat result;
  //double scale_factor = 1.0/4.0;
  //resize(image,result, cv::Size(), scale_factor, scale_factor, cv::INTER_LINEAR);

  cv::Mat imgHSV, threshold_output;
  cv::cvtColor(image, imgHSV, cv::COLOR_BGR2HSV);

  //cvtColor(frame, imgHSV, COLOR_BGR2HSV);
  //cv::inRange(imgHSV, lower, higher, threshold_output);
  cv::inRange(imgHSV, lower_bound, upper_bound, threshold_output);
  //imshow("Thresholded Image", img_thresh);
  return threshold_output;
}


/*

histogram_class::histogram_class():my_bucket_count(0){};
histogram_class::~histogram_class(){};

histogram_class::histogram_class(cv::Mat image):my_bucket_count(0){
  my_image = image.clone();
}

void histogram_class::set_image(cv::Mat image){
  my_image = image.clone();
  my_bucket_count = 0;
}

cv::Mat histogram_class::get_data1(const int &bucket_count){
  // if the data was calculated before then then bucketcounts match
  std::cout << "combine the histo data from the 3 channels" << std::endl;
  if (my_bucket_count == bucket_count) return b_hist; //, g_hist, r_hist;

  // if new call then recalculate
  calc_histogram_data();
  calc_histogram_plot();
  my_bucket_count = bucket_count;

  std::cout << "done calculating the data" << std::endl;
  std::cout << b_hist << std::endl;
  return b_hist; //, g_hist, r_hist;
}

cv::Mat histogram_class::get_plot(const int &bucket_count){

  std::cout << "histo : get_plot()" << std::endl;

  // if the data was calculated before then then bucketcounts match
  if (my_bucket_count == bucket_count) {
    std::cout << "returning plot from earlier" << std::endl;
    return histogram_plot;
  }

  // if new call then recalculate
  calc_histogram_data();
  calc_histogram_plot();
  my_bucket_count = bucket_count;

  std::cout << "done calculating the plot" << std::endl;
  std::cout << b_hist << std::endl;

  return histogram_plot;
}

bool histogram_class::calc_histogram_data(){

  if (my_bucket_count == 0) return false;

  std::vector<cv::Mat> bgr_planes;
  cv::split(my_image, bgr_planes);

  float range[] = {0, 256};
  const float *histRange = {range};

  bool uniform = true;
  bool accumulate = false;

  cv::calcHist(&bgr_planes[0], 1, 0, cv::Mat(), b_hist, 1, &my_bucket_count,
               &histRange, uniform, accumulate);
  cv::calcHist(&bgr_planes[1], 1, 0, cv::Mat(), g_hist, 1, &my_bucket_count,
               &histRange, uniform, accumulate);
  cv::calcHist(&bgr_planes[2], 1, 0, cv::Mat(), r_hist, 1, &my_bucket_count,
               &histRange, uniform, accumulate);

  return true;
}

cv::Mat histogram_class::get_data(const cv::Mat image, int channel){

  //cv::Mat rgba( 100, 100, CV_8UC4, cv::Scalar(1,2,3,4) );
  //Mat bgr( rgba.rows, rgba.cols, CV_8UC3 );
  //cv::Mat alpha( rgba.rows, rgba.cols, CV_8UC1 );


  // forming an array of matrices is a quite efficient operation,
  // because the matrix data is not copied, only the headers
  //Mat out[] = { bgr, alpha };
  // rgba[0] -> bgr[2], rgba[1] -> bgr[1],
  // rgba[2] -> bgr[0], rgba[3] -> alpha[0]
  //int from_to[] = { 0,2, 1,1, 2,0, 3,3 };

  int channel_count = image.channels();
  cv::Mat result( image.rows, image.cols, CV_8UC1 );
  int from_to[] = { channel, 0 };

  // std::cout << "extracting channel " << channel+1 << " of " << channel_count << std::endl;
  // std::cout << image.size().height << " , " << image.size().width << " , " << image.channels() << std::endl;
  // std::cout << result.size().height << " , " << result.size().width << " , " << result.channels() << std::endl;
  // std::cout << image.size() << std::endl;
  // std::cout << result.size() << std::endl;



  int use_this_channel_count = 1; // the function expects an array of this size, the mat has 3 channels but they are "internal"
  cv::mixChannels( &image, use_this_channel_count, &result, 1, from_to, 1 );
  return result;
}

cv::Mat histogram_class::filter_hsv_for_color(const cv::Mat &image){

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

  return imgHSV;

  //cvtColor(frame, imgHSV, COLOR_BGR2HSV);
  //cv::inRange(imgHSV, lower, higher, threshold_output);
  //imshow("Thresholded Image", img_thresh);
  return threshold_output;
}

cv::Mat histogram_class::scale_image(cv::Mat &image){
  double min, max;
  minMaxLoc(image, &min, &max);
  //std::cout << min << " , " << max << std::endl;
  image.convertTo(image, CV_32F, 1/max, -min);
  minMaxLoc(image, &min, &max);
  //std::cout << min << " , " << max << std::endl;
  return image;
}


cv::Mat histogram_class::add_channels(const cv::Mat &image){

  if (image.channels() !=3 ) std::cout << "add_channels input image channel count != 3" << std::endl;
  if (image.channels() ==1 ) {
    std::cout << "add_channels input image channel count == 1, returning input image" << std::endl;
    return image;
  }

  // split the image into its channels
  cv::Mat different_Channels[3];
  cv::split(image, different_Channels);
  //return different_Channels[2];

  // process the channels
  cv::Mat result( image.rows, image.cols, CV_8UC1 );

  // debug output before operations
  std::cout << image.size() << "," << result.size() << "," << different_Channels[0].size() << std::endl;
  std::cout << image.channels() << "," << result.channels() << "," << different_Channels[0].channels() << std::endl;

  //addWeighted(image, alpha, overlay, beta, gamma, result);
  addWeighted(different_Channels[0], 1.0, different_Channels[1], 1.0, 0.0, result);
  addWeighted(result, 1.0, different_Channels[2], 0.0, 0.0, result);
  return result;
}



bool histogram_class::calc_histogram_plot(){

  if (my_bucket_count == 0) return false;

  //drawHistogram(b_hist,g_hist,r_hist);
  //void drawHistogram(cv::Mat& b_hist,cv::Mat& g_hist,cv::Mat& r_hist)

  int hist_w = 512;
  int hist_h = 400;
  int bin_w = cvRound((double)hist_w / my_bucket_count);

  cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0, 0, 0));
  cv::normalize(b_hist, b_hist, 0, histImage.rows, cv::NORM_MINMAX, -1,
                cv::Mat());
  cv::normalize(g_hist, g_hist, 0, histImage.rows, cv::NORM_MINMAX, -1,
                cv::Mat());
  cv::normalize(r_hist, r_hist, 0, histImage.rows, cv::NORM_MINMAX, -1,
                cv::Mat());

  for (int i = 1; i < my_bucket_count; i++) {
    cv::line(
        histImage,
        cv::Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
        cv::Point(bin_w * (i), hist_h - cvRound(b_hist.at<float>(i))),
        cv::Scalar(255, 0, 0), 2, 8, 0);
    cv::line(
        histImage,
        cv::Point(bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
        cv::Point(bin_w * (i), hist_h - cvRound(g_hist.at<float>(i))),
        cv::Scalar(0, 255, 0), 2, 8, 0);
    cv::line(
        histImage,
        cv::Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
        cv::Point(bin_w * (i), hist_h - cvRound(r_hist.at<float>(i))),
        cv::Scalar(0, 0, 255), 2, 8, 0);
  }

  std::cout << "avoid the later image assignment in histo plot" << std::endl;
  histogram_plot = histImage;
  return true;
}

*/
