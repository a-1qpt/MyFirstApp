cmake_minimum_required(VERSION 3.22.1)
project("DD")
include_directories(Eigen)
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp
        IMU_trans.cpp
        my_filtfilt.cpp
        filtfilt.h
        IMU_process.h)
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log)
