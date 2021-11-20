cmake_minimum_required(VERSION 3.13)
project(hsv_contours)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} )

add_library(libfileops STATIC
    ../../common_code/file_operations.cpp
)

add_library(libimageops STATIC
    ../../common_code/image_operations.cpp
)

add_library(libdisplaymanager STATIC
    ../../common_code/display_manager_class.cpp
)

target_include_directories(libfileops PUBLIC ../common_code/ )
target_include_directories(libimageops PUBLIC ../common_code/ )
target_include_directories(libdisplaymanager PUBLIC ../common_code/ )

add_executable(cones_hsv_contour.exe cones_hsv_filter_and_contour_discovery.cpp)

target_link_libraries(cones_hsv_contour.exe libfileops libimageops libdisplaymanager ${OpenCV_LIBS})
