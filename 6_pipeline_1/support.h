#ifndef SUPPORT_OPS
#define SUPPORT_OPS

#include "opencv2/opencv.hpp"

bool is_orange(cv::Vec3b color);

const std::vector<cv::Rect> init_view_windows{ cv::Rect(00,200,300,200), cv::Rect(200,100,100,100), cv::Rect(100,200,50,200) };

class node_class{
public:
  cv::Point point;
  node_class *prev, *next;

  node_class();
  node_class(cv::Point point);
  node_class(cv::Point point, node_class *prev, node_class *next);
  ~node_class();
};

class spider_web_class{
public:
  spider_web_class();
  spider_web_class(int start_y, int start_x, int max_x, int max_y, int distance);
  ~spider_web_class();
  void create_web(int start_x, int start_y, int max_x, int max_y, int distance);

  cv::Point restart();
  cv::Point get_next();

private:
  node_class *start_node;
  node_class *current_node;
  node_class* make_web(int sx, int sy, int ex, int ey, int distance, node_class *current_node);
};

/*
class initRoi {
private:
  int start_x, start_y;
  int end_x, end_y;
  int status;
  cv::Rect selected_roi;

public:
  initRoi():status(0){};
  void init(int x, int y) { start_x = x; start_y = y; end_x = x; end_y = y; status = 1; selected_roi = cv::Rect(); }; // reset end point or a rectangle to previous endpoint maybe displayed temporarily
  void update(int x, int y) { if (status == 1) { end_x = x; end_y = y; } }; // only if a selection was started track the endpoint
  void finish(int x, int y) { end_x = x; end_y = y; status = 2; selected_roi = cv::Rect( start_x, start_y, end_x - start_x, end_y - start_y); }; // selection complete, record the rectangle
  void reset(){ start_x = 0; start_y = 0; end_x = 0; end_y = 0; status = 0; selected_roi = cv::Rect(); }; // selection saved, restart the object
  int get_status() { return status; };
  cv::Rect get_roi(){
    if (status == 2) return selected_roi;  // if selection was completed return the selection
    else if (status == 1) return cv::Rect( start_x, start_y, end_x - start_x, end_y - start_y); // if selection was started, return the current
    else return cv::Rect();
  };
};

initRoi SelectedRoi;

static void mouse_call_back_function(int event, int x, int y, int flags, void* img){
  //Mouse Right button down
  if (event == cv::EVENT_RBUTTONDOWN) {
    //std::cout << "right button " << std::endl;
    return;
  }
  //Mouse Left button down
  if (event == cv::EVENT_LBUTTONDOWN) {
    SelectedRoi.init(x,y);
    //std::cout << "left button DOWN" << std::endl;
    return;
  }
  //Mouse Left button up
  if (event == cv::EVENT_LBUTTONUP) {
    SelectedRoi.finish(x,y);
    //std::cout << "left button UP" << std::endl;
    return;
  }
  //Mouse move coordinates update
  if (event == cv::EVENT_MOUSEMOVE) {
    //std::cout << "event mouse move"<< std::endl;
    SelectedRoi.update(x,y);
    return;
  }
}
*/

#endif
