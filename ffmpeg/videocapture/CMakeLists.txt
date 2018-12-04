cmake_minimum_required(VERSION 3.10)
project(videoCapture)

include_directories(/usr/local/include/)

link_libraries(/usr/local/lib/libavutil.dylib)
link_libraries(/usr/local/lib/libavformat.dylib)
link_libraries(/usr/local/lib/libavfilter.dylib)
link_libraries(/usr/local/lib/libavdevice.dylib)
link_libraries(/usr/local/lib/libavcodec.dylib)

add_executable(videoCapture main.cpp)