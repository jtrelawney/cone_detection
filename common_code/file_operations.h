#ifndef FILEOPS
#define FILEOPS

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include "opencv2/opencv.hpp"

#include <filesystem>
using path = std::filesystem::path;

std::string get_time_stamp(const path &this_path);
bool compare_image_filenames(const path &path1, const path &path2);
std::vector<path> read_image_list(const path &video_dir_path);
path extract_filename(const path &this_path);

class image_reader{
public:
  image_reader(std::filesystem::path image_directory, cv::ImreadModes image_mode);
  ~image_reader();
  void set_imread_mode(cv::ImreadModes image_mode);
  void set_image_directory(std::filesystem::path image_directory);
  int get_image_count();

  cv::Mat operator[](int index);

private:
  image_reader();

  // define the image mode of the images  gray / color / other
  cv::ImreadModes image_mode;

  std::filesystem::path image_directory;
  std::vector<path> image_list;
};

#endif
