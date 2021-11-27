#include "file_operations.h"

using path = std::filesystem::path;

image_reader::image_reader(std::filesystem::path image_directory, cv::ImreadModes image_mode){

  //if ( (image_mode == cv::IMREAD_GRAYSCALE) | (image_mode == cv::IMREAD_COLOR) )
  this->image_mode = image_mode;

  int N = 0;
  this->image_directory = image_directory;
  bool dir_exists = std::filesystem::is_directory(this->image_directory);

  std::cout << "creating image reader object" << std::endl;
  if (dir_exists == true) {
    this->image_list = read_image_list(image_directory);
    N = this->image_list.size();
    std::cout << "image dir = " << this->image_directory << " exists with " << N << " images." << std::endl;
  } else std::cout << "image dir = " << this->image_directory << " does not exists" << std::endl;
}

cv::Mat image_reader::operator[](int index){
  // read and display the requested image frame
  int image_count = this->image_list.size();
  if ( image_count == 0 ) {
    std::cout << "image list is of size 0, does image directory " << this->image_directory << " exist?" << std::endl;
    assert ( image_count > 0);
  }

  std::filesystem::path image_path = image_list.at(index);
  bool file_exists = std::filesystem::is_regular_file(image_path);
  if (file_exists == false) {
    std::cout << "in image reader [], file = " << image_path << " does not exist!" << std::endl;
    std::cout << "check image directory " << this->image_directory << " and file types." << std::endl;
  }

  if (index > image_count ) {
    std::cout << "found " << image_count << " images, requested " << index << ", continue with max index" << std::endl;
    index = image_count - 1 ;
  }

  //std::cout << "loading image " << index << std::endl;
  cv::Mat image = cv::imread(image_path, image_mode );
  return image;
}

image_reader::image_reader(){}
image_reader::~image_reader(){}
void image_reader::set_imread_mode(cv::ImreadModes image_mode){ this->image_mode = image_mode; } //= cv::IMREAD_GRAYSCALE; //IMREAD_COLOR;//IMREAD_GRAYSCALE; //
void image_reader::set_image_directory(std::filesystem::path image_directory){
  this->image_directory = image_directory;
  this->image_list = read_image_list(image_directory);
}
int image_reader::get_image_count(){ return image_list.size(); }

path extract_filename(const path &this_path){
  return this_path.filename();
//  return std::filesystem::getFileName(this_path);
}


std::string get_time_stamp(const path &this_path){

  // to split the path names into its pieces
  //https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/

  std::string val;
  std::vector<std::string> pieces;
  std::istringstream streamData(this_path);
  while (std::getline(streamData, val, '_')) { pieces.push_back(val); }


  // use the second last and last piece for comparison (timestamp seconds and timestamp millisecs)
  std::string nanos = pieces.back();
  pieces.pop_back();
  std::string seconds = pieces.back();

  // normalize the nanos to equal length (str transformations may have cut some characters)
  size_t length = nanos.length();
  // pad length can be anything longer than the longest existing nano string, including the .jpg at the end
  size_t pad_length = 12+4;
  if (length < pad_length)
    nanos.insert(0, pad_length - length, '0');

  std::string timestamp = seconds + nanos;

  //std::cout << this_path << "," << seconds << "," << nanos << "," << timestamp << std::endl;
  return timestamp;
}

bool compare_image_filenames(const path &path1, const path &path2){
  return get_time_stamp(path1) < get_time_stamp(path2);
}


std::vector<path> read_image_list(const path &video_dir_path){
  std::vector<path> images;
  std::cout << "looking for files in " << video_dir_path << std::endl;

  for (const auto & entry : std::filesystem::directory_iterator(video_dir_path)) {
    //std::cout << entry.path() << std::endl;
    images.push_back(entry.path());
  }
  sort(images.begin(), images.end(),compare_image_filenames);
  return images;
}
