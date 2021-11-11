#include "file_operations.h"

using path = std::filesystem::path;

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
