cmake_minimum_required(VERSION 2.6)
project(videoRecorder)
add_library(vvcl SHARED vvcl.cpp)
target_link_libraries(vvcl avcodec avformat avutil swscale)
