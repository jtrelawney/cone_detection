#include "support.h"

extern const std::vector<cv::Rect> init_view_windows;

#include "file_operations.h"
#include "image_operations.h"
#include "display_manager_class.h"
#include "opencv2/opencv.hpp"
//#include "assert";
//#include "cassert";

//using namespace cv;

//using namespace cv::ml;

#include <iostream>

// global variables, necessary to make the image sliders work
//hsv parameters preset
int iLowH = 0;
int iHighH = 70;
int iLowS = 100;
int iHighS = 110, track_cone_high;
int iLowV = 80, track_cone_low;
int iHighV = 200;

// call back functions for the sliders, only necessary when they should ajdust / display something
static void tb_a_low( int, void* ){}
static void tb_a_high( int, void* ){}
static void tb_b_low( int, void* ){}
static void tb_b_high( int, void* ){}
static void tb_c_low( int, void* ){ track_cone_low = iLowV; }
static void tb_c_high( int, void* ){ track_cone_high = iHighS; }




int main(int, char**)
{
  // main window displays the original camera image and the sliders
  cv::namedWindow("original", cv::WINDOW_AUTOSIZE);

  //createTrackbar( variable name on slider , window , &variable, slider_max, callback function);
  //createTrackbar( "threshold", "original", &iLowH, 100, tb_a_low );
  //createTrackbar( "channel", "original", &iHighH, 3, tb_a_high );
  //createTrackbar( "background_high", "original", &iHighH, 255, tb_b_high );
  //createTrackbar( "background_low", "original", &iLowS, 255, tb_b_low );
	//createTrackbar( "cone_high", "original", &iHighS, 255, tb_c_high );
	//createTrackbar( "cone_low", "original", &iLowV, 255, tb_c_low );

  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // read the list with cone patterns, one will be used to find the cone in the image
  std::filesystem::path cone_pattern_dir_path = "../../cone_templates";
  std::vector<path> pattern_list;
  pattern_list = read_image_list(cone_pattern_dir_path);

  // opencv global variables
  cv::Mat frame, display_image, cone_template, pre_processed_frame;

  // index for the images in the video and index for the various cone template images
  int frame_counter = 0;
  int pattern_counter = 0;

  // define which mode the images are read from file
  // for this app we work with grayscale
  cv::ImreadModes image_mode = cv::IMREAD_COLOR; //;//IMREAD_GRAYSCALE; //

  cv::namedWindow("main", cv::WINDOW_AUTOSIZE);
  display_manager window_man(4);

  spider_web_class web;
  std::vector<WEB_PT_TYPE> orange_points;
  cv::Rect init_roi;

  // until press escape
  while(1){

    // read and display the current cone pattern
    //path pattern_path = pattern_list.at(pattern_counter);
    //cv::cone_template = imread(pattern_path, image_mode );
    //cone_template = create_cone_template();
    //cv::cone_template =  cv::Scalar::all(255) - cone_template;
    //cv::imshow("template", cone_template);

    // read and display the current image frame
    path image_path = image_list.at(frame_counter);
    //std::cout << "loading image " << frame_counter << std::endl;
    frame = imread(image_path, image_mode );
    display_image = frame.clone();

    // at first iteration configure the window manager and other stuff
    if (window_man.is_configured==false) {
      window_man.configure(frame);
      window_man.set_window("original",0);
      window_man.set_window("filter",1);
      window_man.set_window("match",2);
      window_man.set_window("result",3);

      // region of interest where expect to find cones
      init_roi = cv::Rect(0,200,frame.cols+1,125);

      // create the web of points to check for orange
      //web.create_web(10, 10, 100, 100, 30);
      //web.create_web(10, 10, frame.cols - 10, frame.rows - 10, 10);
      web.create_web(init_roi.x, init_roi.y, init_roi.x + init_roi.width, init_roi.y + init_roi.height,10);

    }

    // convert the frame to hsv
    cv::Mat imgHSV;
    cv::cvtColor(frame, imgHSV, cv::COLOR_BGR2HSV);

    //print_image_info(frame,"frame");
    //cv::waitKey(0);

    orange_points.clear();
    std::vector<WEB_PT_TYPE> train_data;
    std::vector<int> train_labels;

    // find orange points in the search web
    WEB_PT_TYPE current = web.restart();
    //std::cout << "starting with " << current << std::endl;

    while (current != WEB_PT_TYPE()){
      //std::cout << "circling " << current << std::endl;
      cv::Vec3b color = imgHSV.at<cv::Vec3b>(current);
      //std::cout << "color " << color << std::endl;

      // mark the orange points in the web and prepare the knn data,labels
      if (is_orange(color)) {
        cv::circle(display_image, current, 5, cv::Scalar(0,0,255), 1);
        train_data.push_back(current);
        train_labels.push_back(1);
      } else {
        cv::circle(display_image, current, 3, cv::Scalar(255,255,255), 1);
        orange_points.push_back(current);
        train_data.push_back(current);
        train_labels.push_back(0);
      }
      current = web.get_next();
    }

    // convert to mat
    cv::Mat td(train_data.size(), 2, CV_64F, train_data.data());
    cv::Mat tl(train_labels.size(), 1, CV_32S, train_labels.data());

    bool debug = false;

    if (debug==true){

    // debug output, lots of shit due to varying datatypes and conversion troubles
    //make sure the knn data looks ok
    std::cout << "\n\ntest output" << std::endl;

    std::cout << "\n\nhead of train data vector" << std::endl;
    auto train_test_vec = slice(train_data,0,5);
    for (auto t: train_test_vec) std::cout << t << std::endl;

    // convert to mat and check on proper conversion
    //lots can go wrong: need exact size (rows and cols), datatypes need to match, and no channel defs
    std::cout << "\n\nhead of train mat" << std::endl;
    cv::Mat train_test_mat = cv::Mat(train_test_vec.size(),2,CV_64F,train_test_vec.data());
    print_image_info(train_test_mat,"data");

    // access the elements directly
    std::cout << "head of train data mat, cell access" << std::endl;
    for (int i=0; i<train_test_mat.rows; i++){
      auto x = train_test_mat.at<double>(i,0);
      auto y = train_test_mat.at<double>(i,1);
      std::cout << x << "," << y << std::endl;
    }

    // production traindaata as mat
    std::cout << "\n\nhead of production train data mat" << std::endl;
    std::cout << td(cv::Rect(0,0,2,5)) << std::endl;

    // and direct access
    std::cout << "head of train data mat, cell access" << std::endl;
    for (int i=0; i<4; i++){
      int x = td.at<double>(i,0);
      int y = td.at<double>(i,1);
      std::cout << x << "," << y << std::endl;
    }

    std::cout << "\n\nhead of test label data vector" << std::endl;
    auto label_test_vec = slice(train_labels,0,5);
    for (auto t: label_test_vec) std::cout << t << std::endl;

    std::cout << "head of production label data mat" << std::endl;
    std::cout << tl(cv::Rect(0,0,1,5)) << std::endl;

    double mini,maxi;
    cv::minMaxLoc(tl,&mini,&maxi);
    std::cout << "min and max of label values, shoudl be 0 and 1" << std::endl;
    std::cout << mini << "," << maxi << std::endl;

    print_image_info(td,"data");
    print_image_info(tl,"label");
  }

    td.convertTo(td,CV_32F);
    tl.convertTo(tl,CV_32F);

  if (debug==true) {
    print_image_info(td,"data");
    print_image_info(tl,"label");

    std::cout << "\n\nhead of production train data mat after type conversion to 32F for KNN" << std::endl;
    std::cout << td(cv::Rect(0,0,2,5)) << std::endl;
    std::cout << "head of production label data mat after type conversion to 32F for KNN" << std::endl;
    std::cout << tl(cv::Rect(0,0,1,5)) << std::endl;

    // for (int i=0; i<train_data.size(); i++){
    //   int x = td.at<float>(i,0);
    //   int y = td.at<float>(i,1);
    //   auto o = tl.at<float>(i);
    //   int r = 0;
    //   if (o!=0) std::cout << i << " : " << x << "," << y << "   : " << o << std::endl; //<< "," << r << "," << std::endl;
    // }
    cv::waitKey(0);
  }

    // create the data object
    cv::Ptr<cv::ml::TrainData> trainingData;
    trainingData = cv::ml::TrainData::create(td,cv::ml::ROW_SAMPLE,tl);

    // create the knn object
    cv::Ptr<cv::ml::KNearest> knn = cv::ml::KNearest::create();
    knn->setIsClassifier(0);
    knn->setAlgorithmType(cv::ml::KNearest::BRUTE_FORCE);
    knn->setDefaultK(4);
    //knn->train(td, cv::ml::ROW_SAMPLE, tl);
    knn->train(trainingData);

    // predict
    cv::Mat response, dist;
    knn->findNearest(td, 1, cv::noArray(), response, dist);

    //print_image_info(response,"res");

    for (int i=0; i<td.rows; i++){
      int x = td.at<float>(i,0);
      int y = td.at<float>(i,1);
      float o = tl.at<float>(i);
      float r = response.at<float>(i);

      int ss = 2;
      cv::Point ll(x-ss,y-ss), ur(x+ss,y+ss);

      if (debug == true) std::cout << i << " : " << x << "," << y << "   : " << o << "," << r << "," << std::endl;

      if (o!=0) {
        cv::rectangle(frame, ll, ur, cv::Scalar(0,255,0) );
      }
    }

    if (debug){
      std::cout << "prediction and error distance " << std::endl;
      std::cout << response(cv::Rect(0,0,1,5)) << std::endl;
      std::cout << dist(cv::Rect(0,0,1,5)) << std::endl;
    }

    //cv::waitKey(0);

    //draw to roi into the image
    cv::rectangle(
      frame,
      init_roi,
      cv::Scalar(0, 0, 255),1, 8, 0
    );

    imshow("original", frame);
    window_man.set_image("result", frame);
    window_man.set_image("filter", display_image);
    cv::Mat fucker = window_man.get_image();
    imshow("main",fucker);

    // wait for button press
    // escape = end
    // space = next frame
    // meanwhile use the sliders
    char c = (char) cv::waitKey(25);

    // esc ends the show
    if(c==27) // esc
      break;
    // next frame
    else if (c==32) {
      if (frame_counter < image_list.size()) frame_counter++; else frame_counter = 0;
    }
  }

  return 0;
}
