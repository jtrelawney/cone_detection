#ifndef SUPPORT_OPS
#define SUPPORT_OPS

#include "opencv2/opencv.hpp"

bool is_orange(cv::Vec3b color);

template<typename T>
std::vector<T> slice(std::vector<T> const &v, int m, int n)
{
  if (n<m) {int help = m; m=n; n=help;};
  // ensure max entries
  if (n > v.size()+1) n = v.size()-1;
  auto first = v.cbegin() + m;
  auto last = v.cbegin() + n + 1;

  std::vector<T> vec(first, last);
  return vec;
};

//typedef std::vector<std::vector<cv::Point>> blob_cloud;
using blob_cloud = std::vector<std::vector<cv::Point2i>>;

const std::vector<cv::Rect> init_view_windows{ cv::Rect(00,200,300,200), cv::Rect(200,100,100,100), cv::Rect(100,200,50,200) };

class blobs{
public:
  blobs();
  ~blobs();
  int add_point(cv::Point point);
  blob_cloud get_blobs();

private:
  blob_cloud my_blobs;
  int blob_count;
};

using WEB_PT_TYPE = cv::Point2d;

class node_class{
public:
  WEB_PT_TYPE point;
  node_class *prev, *next;

  node_class();
  node_class(WEB_PT_TYPE point);
  node_class(WEB_PT_TYPE point, node_class *prev, node_class *next);
  ~node_class();
};

class spider_web_class{
public:
  spider_web_class();
  spider_web_class(int start_y, int start_x, int max_x, int max_y, int distance);
  ~spider_web_class();
  void create_web(int start_x, int start_y, int max_x, int max_y, int distance);

  WEB_PT_TYPE restart();
  WEB_PT_TYPE get_next();

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
