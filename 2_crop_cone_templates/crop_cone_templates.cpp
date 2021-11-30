#include <iostream>

#include "file_operations.h"
#include "opencv2/opencv.hpp"
using namespace cv;

// crops a rectangle from the original image
cv::Mat crop_cone_from_image(const cv::Mat &image, const int x, const int y, const int width, const int height){
  Mat ROI(image, Rect(x,y,width,height));
  Mat croppedImage;
  ROI.copyTo(croppedImage);
  return croppedImage;
}

// based on idea of https://funvision.blogspot.com/2020/01/opencv-mouse-drawing-c-tutorial-mouse.html
class initRoi {
private:
  int start_x, start_y;
  int end_x, end_y;
  int status;
  cv::Rect selected_roi;

public:
  initRoi():status(0){};
  void init(int x, int y) { start_x = x; start_y = y; end_x = x; end_y = y; status = 1; selected_roi = Rect(); }; // reset end point or a rectangle to previous endpoint maybe displayed temporarily
  void update(int x, int y) { if (status == 1) { end_x = x; end_y = y; } }; // only if a selection was started track the endpoint
  void finish(int x, int y) { end_x = x; end_y = y; status = 2; selected_roi = Rect( start_x, start_y, end_x - start_x, end_y - start_y); }; // selection complete, record the rectangle
  void reset(){ start_x = 0; start_y = 0; end_x = 0; end_y = 0; status = 0; selected_roi = Rect(); }; // selection saved, restart the object
  int get_status() { return status; };
  cv::Rect get_roi(){
    if (status == 2) return selected_roi;  // if selection was completed return the selection
    else if (status == 1) return Rect( start_x, start_y, end_x - start_x, end_y - start_y); // if selection was started, return the current
    else return cv::Rect();
  };
} SelectedRoi;

static void mouse_call_back_function(int event, int x, int y, int flags, void* img){
  //Mouse Right button down
  if (event == EVENT_RBUTTONDOWN) {
    //std::cout << "right button " << std::endl;
    return;
  }
  //Mouse Left button down
  if (event == EVENT_LBUTTONDOWN) {
    SelectedRoi.init(x,y);
    //std::cout << "left button DOWN" << std::endl;
    return;
  }
  //Mouse Left button up
  if (event == EVENT_LBUTTONUP) {
    SelectedRoi.finish(x,y);
    //std::cout << "left button UP" << std::endl;
    return;
  }
  //Mouse move coordinates update
  if (event == EVENT_MOUSEMOVE) {
    //std::cout << "event mouse move"<< std::endl;
    SelectedRoi.update(x,y);
    return;
  }
}

int main(int, char**)
{

  std::cout << "extract cone template" << std::endl;
  //std::cout << "left = a, right = d, up = w, down = x, width = r,t , height = h,n,   save file = s" << std::endl;
  std::cout << "left mouse button down - start, left mouse button up - select, key s = save" << std::endl;

  // main window displays the original camera image and the sliders
  namedWindow("original", WINDOW_AUTOSIZE);

  setMouseCallback("original", mouse_call_back_function, 0);

  // read the list with filenames for sequential camera frames
  std::filesystem::path video_dir_path = "../../cone_movies/run4";
  std::vector<path> image_list;
  image_list = read_image_list(video_dir_path);

  // opencv global variables
  Mat frame;//, display_image; //imgHSV, img_thresh;
  int frame_counter = 0;

  // until press escape
  while(1){

    // read and display the current image
    path image_path = image_list.at(frame_counter);
    frame = imread(image_path, IMREAD_COLOR );
    cv::Mat frame_copy = frame.clone();

    // if selection was complete - draw a red ROI
    // if selection was initiated - draw a white ROI
    switch (SelectedRoi.get_status()){
      case 0:
        break;
      case 1:
        rectangle(
          frame,
          SelectedRoi.get_roi(),
          Scalar(255, 255, 255),1, 8, 0
        );
        break;
      case 2:
        rectangle(
          frame,
          SelectedRoi.get_roi(),
          Scalar(0,0, 255), 1, 8, 0
        );
        break;
      }

    imshow("original", frame);

    // wait for button press
    // escape = end
    // space = next frame
    // s = save selected area
    char c = (char) waitKey(25);

    // esc ends the show
    if(c==27) // esc
      break;
    // next frame
    else if (c==32) {
      if (frame_counter < image_list.size()) frame_counter++; else frame_counter = 0;
    }
    // s = save image
    else if (c==115){

      // save roi in case selection was completed
      if (SelectedRoi.get_status() == 2){
        cv::Rect roi = SelectedRoi.get_roi();
        std::string file_quali = "./cropped_" + std::to_string(roi.x) + "_" + std::to_string(roi.y) + "_" + std::to_string(roi.width)+ "_" + std::to_string(roi.height) + "_";
        std::string template_filename =  file_quali + image_path.filename().string();
        path template_path(template_filename);
        std::cout << "saving image" << template_path << std::endl;

        // crop from the copy or the on screen rectangle will be cropped too
        Mat cropped = frame_copy(roi);
        cv::imshow("cropped image ",cropped);
        imwrite(template_path, cropped);
        cv::waitKey(0);

        SelectedRoi.reset();
      }
    }
  }

  return 0;
}
