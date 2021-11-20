#include "feature_detection.h"


feature_detection_class::feature_detection_class(){
  //detector = cv::AKAZE::create(); // image downsize by 8 doesn't find features
  ///detector = cv::ORB::create(); // image downsize by 8 doesn't find features
  //detector = cv::SURF::create(); // rquires nonfree
  detector = cv::SIFT::create();  // finds features in cone image but mostly in teh background
  detection_complete = false;
}

feature_detection_class::~feature_detection_class(){
}

bool feature_detection_class::extract_keypoints(cv::Mat image){
  //detector->detectAndCompute(image, cv::noArray(), keypoints, descriptor);
  detector->detect(image, keypoints, cv::Mat());
  detection_complete = true;
  std::cout << keypoints.size() << " keypoints extracted" << std::endl;
  return detection_complete;
}

bool feature_detection_class::draw_keypoints(cv::Mat &image){
  if (detection_complete == false) {
    std::cout << "before drawing keypoints run detection first" << std::endl;
    return detection_complete;
  }

  std::cout << "plotting " << keypoints.size() << " keypoints" << std::endl;
  cv::drawKeypoints( image, keypoints, image, cv::Scalar::all(-1), cv::DrawMatchesFlags::DEFAULT );

  return detection_complete;
}
