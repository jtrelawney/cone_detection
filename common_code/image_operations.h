#ifndef IMAGE_OPS
#define IMAGE_OPS

#include <iostream>
#include "opencv2/opencv.hpp"
#include <string>
#include <map>

// provides wrapper functions to the key opencv functionality

bool get_histogram(const cv::Mat &image, const int &bucket_count, cv::Mat &result_image, cv::Mat &result_data);
void put_text(cv::Mat &image, std::string this_text, int x, int y, cv::Scalar font_color);
cv::Mat filter_hsv_for_color(const cv::Mat &image, cv::Scalar lower_bound, cv::Scalar upper_bound);
std::vector<cv::Point> find_local_minima(const cv::Mat &image);
cv::Mat crop_area_from_image(const cv::Mat &image, const int x, const int y, const int width, const int height);
cv::Mat create_cone_template(bool reverse_background=false);
cv::Mat heatmap_from_template_match(const cv::Mat &img, const cv::Mat &templ, const cv::TemplateMatchModes &match_method);

void print_image_info(const cv::Mat &image, std::string mat_text);
#endif
