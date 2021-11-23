cmake_minimum_required(VERSION 3.13)
project(v_pts_2_mat)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} )

#add_library(libfileops STATIC
#    ../../common_code/file_operations.cpp
#)

#target_include_directories(libfileops PUBLIC ../common_code/ )

add_executable(v_pts_2_mat.exe vector_cv_point_2_mat_conversion.cpp)

target_link_libraries(v_pts_2_mat.exe ${OpenCV_LIBS})
