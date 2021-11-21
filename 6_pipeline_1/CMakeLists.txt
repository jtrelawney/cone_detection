cmake_minimum_required(VERSION 3.13)
project(pipeline1)

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

add_library(libsupport STATIC
    ../support.cpp
)

#add_library(libfeaturedetect STATIC
#    ../../common_code/feature_detection.cpp
#)

target_include_directories(libfileops PUBLIC ../common_code/ )
target_include_directories(libimageops PUBLIC ../common_code/ )
target_include_directories(libdisplaymanager PUBLIC ../common_code/ )
target_include_directories(libsupport PUBLIC ../common_code/ )
#target_include_directories(libfeaturedetect PUBLIC ../common_code/ )

add_executable(pipeline_1.exe pipeline_1.cpp)

target_link_libraries(pipeline_1.exe libfileops libdisplaymanager libimageops libsupport ${OpenCV_LIBS})
