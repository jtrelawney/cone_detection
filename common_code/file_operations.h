#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>

#include <filesystem>
using path = std::filesystem::path;

std::string get_time_stamp(const path &this_path);
bool compare_image_filenames(const path &path1, const path &path2);
std::vector<path> read_image_list(const path &video_dir_path);
path extract_filename(const path &this_path);
