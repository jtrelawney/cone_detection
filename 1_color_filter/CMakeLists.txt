cmake_minimum_required(VERSION 3.13)
project(cones_color_filter)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} )

add_library(libfileops STATIC
    ../../common_code/file_operations.cpp
)

target_include_directories(libfileops PUBLIC ../common_code/ )

add_executable(cones_color_filter.exe cones_color_filter.cpp)

target_link_libraries(cones_color_filter.exe libfileops ${OpenCV_LIBS})
