#ifndef FEATDETECT
#define FEATDETECT

#include <iostream>
#include "opencv2/opencv.hpp"

class feature_detection_class{

public:

  feature_detection_class();
  ~feature_detection_class();

  bool extract_keypoints(cv::Mat image);
  bool draw_keypoints(cv::Mat &image);

private:
  //cv::SiftFeatureDetector detector;
  //cv::Ptr<cv::AKAZE> akaze;
  cv::Ptr<cv::Feature2D> detector;
  std::vector<cv::KeyPoint> keypoints;
  cv::Mat descriptor;
  bool detection_complete;
  //cv::AkazeFeatureDetector detector;

  //cv::Ptr<FeatureDetector> detector = cv::ORB::create();
};

#endif
