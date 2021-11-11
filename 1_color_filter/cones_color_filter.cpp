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

int main(int, char**)
{
  // main window displays the original camera image and the sliders
  namedWindow("original", WINDOW_AUTOSIZE);
  createTrackbar( "r_low", "original", &iLowH, slider_max, tb_a_low );
  createTrackbar( "r_high", "original", &iHighH, slider_max, tb_a_high );
  createTrackbar( "g_low", "original", &iLowS, slider_max, tb_a_low );
	createTrackbar( "g_high", "original", &iHighS, slider_max, tb_a_high );
	createTrackbar( "b_low", "original", &iLowV, slider_max, tb_a_low );
	createTrackbar( "b_high", "original", &iHighV, slider_max, tb_a_high );

  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // opencv global variables
  Mat frame, imgHSV, img_thresh;
  int frame_counter = 0;

  // preset the hsv filter
  Scalar lower = Scalar(iLowH, iLowS, iLowV);
  Scalar higher = Scalar(iHighH, iHighS, iHighV);

  // until press escape
  while(1){
    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    frame = imread(image_path, IMREAD_COLOR );
    imshow("original", frame);

    // set the hsv filter according to the slider positions (will be set when the slider is touched)
    lower = Scalar(iLowH, iLowS, iLowV);
    higher = Scalar(iHighH, iHighS, iHighV);

    // apply the hsv filter and display the result
    cvtColor(frame, imgHSV, COLOR_BGR2HSV);
    cvtColor(frame, imgHSV, COLOR_BGR2HSV);
    inRange(imgHSV, lower, higher, img_thresh);
    imshow("Thresholded Image", img_thresh);

    // wait for button press
    // escape = end
    // space = next frame
    // meanwhile use the sliders
    char c = (char) waitKey(25);
    if(c==27) // esc
      break;
    else if (c==32) //  43 is +
      if (frame_counter < image_list.size()) frame_counter++; else frame_counter = 0;
  }

  return 0;
}
