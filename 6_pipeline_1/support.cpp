#include "support.h"

blobs::~blobs(){}
blobs::blobs(){blob_count = 0; }
blob_cloud blobs::get_blobs(){ return my_blobs; }
int blobs::add_point(cv::Point point){
  //my_blobs;
  return 0;
}

node_class::node_class(){this->point = WEB_PT_TYPE(), prev = nullptr; next = nullptr;};
node_class::~node_class(){};
node_class::node_class(WEB_PT_TYPE point){this->point = point, prev = nullptr; next = nullptr;};
node_class::node_class(WEB_PT_TYPE data, node_class *prev, node_class *next){
  this->point = data;
  this->prev = prev;
  this->next = next;
}

spider_web_class::spider_web_class(){ start_node = nullptr; current_node = nullptr; }
spider_web_class::~spider_web_class(){};
spider_web_class::spider_web_class(int start_x, int start_y, int max_x, int max_y, int distance){
  create_web(start_x, start_y, max_x, max_y, distance);
}

void spider_web_class::create_web(int start_x, int start_y, int max_x, int max_y, int distance) {
  std::cout <<  "creating spider web with start x,y = " << start_x << "," << start_y;
  std::cout << "   max x, max y = " << max_x << "," << max_y;
  std::cout << "   , d = " << distance << std::endl;

  start_node = new node_class( WEB_PT_TYPE(start_x,start_y) );
  current_node = start_node;
  make_web(start_x, start_y, max_x, max_y, distance, current_node);

  std::cout << "done with the web" << std::endl;
}

node_class* spider_web_class::make_web(int sx, int sy, int mx, int my, int distance, node_class *current_node){

  //std::cout <<  "currently at " << x << "," << y << std::endl;
  int x = current_node -> point.x, y = current_node -> point.y;
  //std::cout <<  "currently at " <<  x << "," << y << std::endl;

  // create the new node and link its prev
  // also link the current to the new
  // then follow up on the next partner

  if (x+distance<mx) { // only if x doesn't cross the max x when taking the next step
    int new_x = x + distance;
    node_class* new_node = new node_class( WEB_PT_TYPE(new_x,y), current_node, nullptr);
    current_node -> next = new_node;
    make_web( sx, sy, mx, my, distance, new_node);
  } else if (y+distance<my) { // only if x doesn't cross the max y when taking the next step
    // start over with x and drop to next y
    x = sx;
    int new_y = y + distance;
    node_class* new_node = new node_class( WEB_PT_TYPE(x,new_y), current_node, nullptr);
    current_node -> next = new_node;
    make_web( sx, sy, mx, my, distance, new_node);
  } else {
    std::cout <<  "skipping " << x << "," << y << std::endl;
  }

  return nullptr;
}

WEB_PT_TYPE spider_web_class::restart(){
  current_node = start_node;
  if (start_node == nullptr) return WEB_PT_TYPE();
  return start_node->point;
};

WEB_PT_TYPE spider_web_class::get_next(){

  WEB_PT_TYPE result = WEB_PT_TYPE();

  if ( current_node->next != nullptr){
    current_node = current_node->next;
    result = current_node->point;
  }

  return result;
};

int hsv_hue_low = 0;//0
int hsv_hue_high = 70;//192
int hsv_sat_low = 100;//0
int hsv_sat_high = 255;//145
int hsv_value_low = 70; //5
int hsv_value_high = 200; //159

cv::Scalar lower_bound = cv::Scalar(hsv_hue_low, hsv_sat_low, hsv_value_low);
cv::Scalar upper_bound = cv::Scalar(hsv_hue_high, hsv_sat_high, hsv_value_high);

bool is_orange(cv::Vec3b hsv_color){
  //int R = hsv_color[0];
  //bool result = (R>15) && (R<75);
  //bool result = (hsv_color>lower_bound) && (hsv_color<upper_bound);
  int R;
  bool result;
  R = hsv_color[0];
  result = (R>hsv_hue_low) && (R<hsv_hue_high);
  R = hsv_color[1];
  result = result && (R>hsv_sat_low) && (R<hsv_sat_high);
  R = hsv_color[2];
  result = result && (R>hsv_value_low) && (R<hsv_value_high);
  return result;
};
