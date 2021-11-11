cmake_minimum_required(VERSION 3.13)
project(cones_color_filter)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#find_package(GST REQUIRED)

add_executable(cones_color_filter.exe cones_color_filter.cpp)
target_link_libraries(cones_color_filter.exe ${OpenCV_LIBS})
