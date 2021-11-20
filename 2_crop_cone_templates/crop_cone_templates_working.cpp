#include <iostream>

#include "file_operations.h"
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
int iHighS = 255;
int iLowV = 70;
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

int main(int, char**)
{

  std::cout << "extract cone template" << std::endl;
  std::cout << "left = a, right = d, up = w, down = x, width = r,t , height = h,n,   save file = s" << std::endl;
  // main window displays the original camera image and the sliders
  namedWindow("original", WINDOW_AUTOSIZE);
  /*
  createTrackbar( "r_low", "original", &iLowH, slider_max, tb_a_low );
  createTrackbar( "r_high", "original", &iHighH, slider_max, tb_a_high );
  createTrackbar( "g_low", "original", &iLowS, slider_max, tb_a_low );
	createTrackbar( "g_high", "original", &iHighS, slider_max, tb_a_high );
	createTrackbar( "b_low", "original", &iLowV, slider_max, tb_a_low );
	createTrackbar( "b_high", "original", &iHighV, slider_max, tb_a_high );
  */

  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // opencv global variables
  Mat frame, display_image; //imgHSV, img_thresh;
  int frame_counter = 0;

  // preset the hsv filter
  Scalar lower = Scalar(iLowH, iLowS, iLowV);
  Scalar higher = Scalar(iHighH, iHighS, iHighV);

  int x=100, y = 100;
  int width = 20, height = 60;

  // until press escape
  while(1){

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    frame = imread(image_path, IMREAD_COLOR );

    display_image = frame.clone();
    cv::Rect area(x, y , width, height);
    cv::rectangle( display_image, area, cv::Scalar(0, 0, 255) );
    imshow("original", display_image);

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
      std::string file_quali = "./cropped_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(width)+ "_" + std::to_string(height) + "_";
      std::string template_filename =  file_quali + image_path.filename().string();
      path template_path(template_filename);
      std::cout << "saving image" << template_path << std::endl;

      Mat cropped = frame(area);
      cv::imshow("cropped image ",cropped);
      imwrite(template_path, cropped);
      cv::waitKey(0);


    }
  }

  return 0;
}
