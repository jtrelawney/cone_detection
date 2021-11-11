#include <iostream>
#include "opencv2/opencv.hpp"
#include <string>
#include <sstream>

#include <filesystem>
using path = std::filesystem::path;

using namespace cv;


std::string get_time_stamp(const path &this_path){

  // to split the path names into its pieces
  //https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/

  std::string val;
  std::vector<std::string> pieces;
  std::istringstream streamData(this_path);
  while (std::getline(streamData, val, '_')) { pieces.push_back(val); }


  // use the second last and last piece for comparison (timestamp seconds and timestamp millisecs)
  std::string nanos = pieces.back();
  pieces.pop_back();
  std::string seconds = pieces.back();

  // normalize the nanos to equal length (str transformations may have cut some characters)
  size_t length = nanos.length();
  // pad length can be anything longer than the longest existing nano string, including the .jpg at the end
  size_t pad_length = 12+4;
  if (length < pad_length)
    nanos.insert(0, pad_length - length, '0');

  std::string timestamp = seconds + nanos;

  //std::cout << this_path << "," << seconds << "," << nanos << "," << timestamp << std::endl;
  return timestamp;
}

bool compare_image_filenames(const path &path1, const path &path2){
  return get_time_stamp(path1) < get_time_stamp(path2);
}

std::vector<path> read_image_list(const path &video_dir_path){
  std::vector<path> images;
  std::cout << "looking for files in " << video_dir_path << std::endl;

  for (const auto & entry : std::filesystem::directory_iterator(video_dir_path)) {
    //std::cout << entry.path() << std::endl;
    images.push_back(entry.path());
  }
  sort(images.begin(), images.end(),compare_image_filenames);
  return images;
}


const int slider_max = 255;
int alpha_slider;
double alpha;
double beta;

//hsv
int iLowH = 0;
int iHighH = 70;
int iLowS = 100;
int iHighS = 255;
int iLowV = 70;
int iHighV = 200;

// rgb
/*
int iLowH = 15;
int iHighH = 52;
int iLowS = 0;
int iHighS = 255;
int iLowV = 75;
int iHighV = 175;
*/

static void tb_a_low( int, void* ){}
static void tb_a_high( int, void* ){}
static void tb_b_low( int, void* ){}
static void tb_b_high( int, void* ){}
static void tb_c_low( int, void* ){}
static void tb_c_high( int, void* ){}

int main(int, char**)
{
  std::filesystem::path video_dir_path = "../../cone_movies/run4";

  namedWindow("original", WINDOW_AUTOSIZE);
  createTrackbar( "r_low", "original", &iLowH, slider_max, tb_a_low );
  createTrackbar( "r_high", "original", &iHighH, slider_max, tb_a_high );
  createTrackbar( "g_low", "original", &iLowS, slider_max, tb_a_low );
	createTrackbar( "g_high", "original", &iHighS, slider_max, tb_a_high );
	createTrackbar( "b_low", "original", &iLowV, slider_max, tb_a_low );
	createTrackbar( "b_high", "original", &iHighV, slider_max, tb_a_high );

  Mat frame, result_image;
  int frame_counter = 0;

  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  Mat imgHSV, img_thresh;

  Scalar lower = Scalar(iLowH, iLowS, iLowV);
  Scalar higher = Scalar(iHighH, iHighS, iHighV);

  while(1){
    path image_path = image_list.at(frame_counter);
    frame = imread(image_path, IMREAD_COLOR );
    imshow("original", frame);


    lower = Scalar(iLowH, iLowS, iLowV);
    higher = Scalar(iHighH, iHighS, iHighV);

    cvtColor(frame, imgHSV, COLOR_BGR2HSV);
    cvtColor(frame, imgHSV, COLOR_BGR2HSV);
    inRange(imgHSV, lower, higher, img_thresh);
    imshow("Thresholded Image", img_thresh);

    char c = (char) waitKey(25);
    if(c==27) // esc
      break;
    else if (c==32) //  43 is +
      if (frame_counter < image_list.size()) frame_counter++; else frame_counter = 0;
  }

  return 0;
}
