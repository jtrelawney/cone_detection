#include <iostream>
#include "opencv2/opencv.hpp"


// provide an empty point as template type to create a vector of that point class
template< typename T>
std::vector<T> get_points(T p, int N=3){

  std::vector<T> pts;

  for (int i=0; i<N; i++){
    T x(i,i);
    pts.push_back(x);
  }

  return pts;
};

int main(){

  // after half a day debugging a total knn training and prediction fail, the error was located in the type of points used for the training data
  // this triggered the idea to test working combinations and the outcome is that only a perfect match between point type and mat type will ever work
  // since the vector of points was created with the generic cv::Point type, it seems only the CV_32S type would have worked

  // 1. the vector size and the mat dimensions have to match or nothing works
  // 2. the point type has to match the mat type exactly or nothing works
  // 3. channels specifications in the mat type wont work, only the general type

  // working combinations
  // Point2d , CV_64F
  // Point2f , CV_32F
  // Point2i , CV_32S
  // Point, CV_32S

  // busted combinations
  // Point2d , CV_32F
  // cv::Point(), ...
  // Point(),  CV_16UC1
  // Point(), CV_16U
  // Point2i(), CV_16U
  // Point2i , CV_64F
  // Point2i, CV_16SC1
  // Point2i, CV_16S

  // get the test points of a particulat point type
  auto pts = get_points(cv::Point());

  // create a mat from it
  int mat_type = CV_32S;
  cv::Mat point_mat = cv::Mat(pts.size(), 2, mat_type, pts.data());

  // compare the output
  std::cout << pts << std::endl;
  std::cout << point_mat << std::endl;

  return 0;
}
