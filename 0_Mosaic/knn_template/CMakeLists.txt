cmake_minimum_required(VERSION 3.13)
project(knn_template)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} )

#add_library(libfileops STATIC
#    ../../common_code/file_operations.cpp
#)

#target_include_directories(libfileops PUBLIC ../common_code/ )

add_executable(knn_template.exe knn_template.cpp)

target_link_libraries(knn_template.exe ${OpenCV_LIBS})
