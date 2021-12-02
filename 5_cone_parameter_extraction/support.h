#ifndef SUPPORT
#define SUPPORT

#include "opencv2/opencv.hpp"
#include "image_operations.h"

// global variables, necessary to make the image sliders work
//hsv parameters preset
int filter_low_tracker = 240, filter_low = 240;

int filter_sat = 110, filter_sat_tracker = 110;
int filter_val = 90, filter_val_tracker = 90;

int hough_threshold_tracker = 46, hough_threshold = 46;

double rho_param = 6;
int rho_tracker = 6;

int theta_tracker = 180;
double theta_param = 3.141/theta_tracker;

bool equalize_y_n;
int template_select = 0;
bool template_onoff;
int binarize_select = 0;
bool binarize_onoff;


// call back functions for the sliders, only necessary when they should ajdust / display something
//static void tb_filter_low( int, void* ){ filter_low = filter_low_tracker; }

static void tb_filter_sat( int, void* ){ filter_sat = filter_sat_tracker; }
static void tb_filter_val( int, void* ){ filter_val = filter_val_tracker; }

static void tb_hough_threshold( int, void* ){ hough_threshold = hough_threshold_tracker; }
static void tb_rho_param( int, void* ){ rho_param = rho_tracker; }
static void tb_theta_param( int, void* ){ theta_param = 3.141/(double)theta_tracker; }


// helper conversions
double to_rad(double alpha) { return 3.141 * alpha / 180.0; }
double to_deg(double alpha) { return alpha * 180.0 / 3.141; }

void plot_cone_line(cv::Mat &image, double result_theta, int result_rho){

  // rho = distance to best matching line given the pointcloud under the given cone_theta
  // from that create 2 points to draw a line
  cv::Point pt1, pt2;

  // find the anchorpoint x0,y0 of the discovered line
  double a = cos(result_theta), b = sin(result_theta);  // unit vectors into each direction
  double x0 = a*result_rho, y0 = b*result_rho;                    // anchorpoint

  //std::cout << "a = cos(t) = " << a << "   b = sin(t) = " << b << std::endl;
  //std::cout << "x0 = " << x0 << "   y0 = " << y0 << std::endl;


  // choose 2 points to draw the line through
  pt1.x = cvRound(x0 + 1000*(-b));
  pt1.y = cvRound(y0 + 1000*(a));
  pt2.x = cvRound(x0 - 1000*(-b));
  pt2.y = cvRound(y0 - 1000*(a));

  cv::circle(image, cv::Point(x0,y0), 3, cv::Scalar(0,255,0), 1 );
  cv::line( image, pt1, pt2, cv::Scalar(0,0,255), 1, cv::LINE_AA);
}


// the caller adjust the theta from real world to opencv angle
// the rho calculation swaps
void find_cone(const cv::Mat filtered_image, double cone_theta, int iterations, double step_size, int &result_rho, double &result_theta){

  // to store the best rho count through all iterations
  int max_index = -1, max_count = -1;

  for (int counter = 0; counter<iterations; counter++){

    double  theta = cone_theta - (int)(iterations/2) * step_size + counter * step_size; // alternates arround the cone_theta
    //std::cout << "c = " << counter << ", theta = " << theta << std::endl;

    // converts the theta from realworld to opencv (0,0) from bottom left to top left and converts to rad
    theta = to_rad(90.0 - theta);
    //cv::waitKey(0);

    // 0 = C1 8U
    // print_image_info(filtered_image,"find_cone");
    // cv::waitKey(0);

    // for binary image values are either 0 or 255
    int threshold = 200;

    // this where the rho (constraint by the given theta) for each of the points are counted
    std::map< int, int > rho_counter;

    // iterate through pixel coordinates
    for ( int i=0; i<filtered_image.cols; i++){
      for ( int j=0; j<filtered_image.rows; j++){
        uchar value = filtered_image.at<uchar>(i,j);
        //std::cout << "found = " << (int)value << std::endl;

        // thresholding necessary, not really in a binary image
        if  (value>threshold){
          // calculate the rho for the current image coordinate, restraint by the given theta
          //int rho = (int) ( (double)i * cos(theta) + (double)j * sin(theta) );

          // to get the correct theta cos and sin are swapped around (equal to swapping the coordinates) to reflect the mx+b in the opencv coordinate system
          int rho = (int) ( (double)i * sin(theta) + (double)j * cos(theta) );

          // increase the existing count for this rho, and create it it doesnt exist yet
          auto entry = rho_counter.find(rho);
          if ( entry == rho_counter.end() ) rho_counter.insert({rho,1}); else  entry->second++;
        }
      }
    } // end of image iterations

    //std::cout << "============================" << std::endl;
    // find the max rho count for the constraint theta in this point cloud
    for (auto ip:rho_counter){
      if (ip.second > max_count) {
        max_count = ip.second;
        max_index = ip.first;
        result_rho = max_index;
        result_theta = theta;
      }
      //std::cout << ip.first << "," << ip.second << "    max = " << max_index << "," << max_count << std::endl;
    } // end of max iterations

  //std::cout << "============================" << std::endl;

//std::cout << "done counter = " << counter << std::endl;
} // end of theta iterations

  // the max index is the rho
  //return max_index;
}

//static void button_template_onoff( int, void* ){ template_onoff = (template_select>0); }
//static void button_binarize_onoff( int, void* ){ binarize_onoff = (binarize_select>0); }

#endif
