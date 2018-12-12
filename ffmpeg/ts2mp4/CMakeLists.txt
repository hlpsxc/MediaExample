cmake_minimum_required(VERSION 3.10)
project(ts2mp4)



MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

IF (WIN32)
    MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
    include_directories(/usr/local/Cellar/ffmpeg/4.1/include/)
    link_libraries(/usr/local/Cellar/ffmpeg/4.1/lib/libavutil.dylib)
    link_libraries(/usr/local/Cellar/ffmpeg/4.1/lib/libavformat.dylib)
    link_libraries(/usr/local/Cellar/ffmpeg/4.1/lib/libavfilter.dylib)
    link_libraries(/usr/local/Cellar/ffmpeg/4.1/lib/libavdevice.dylib)
    link_libraries(/usr/local/Cellar/ffmpeg/4.1/lib/libavcodec.dylib)
    MESSAGE(STATUS "Now is Apple systens.")
ELSEIF (UNIX)
    include_directories(/usr/local/include/)
    link_libraries(/usr/local/lib/libavutil.so)
    link_libraries(/usr/local/lib/libavformat.so)
    link_libraries(/usr/local/lib/libavfilter.so)
    link_libraries(/usr/local/lib/libavdevice.so)
    link_libraries(/usr/local/lib/libavcodec.so)
    MESSAGE(STATUS "Now is UNIX-like OS's.")
ENDIF ()

add_executable(ts2mp4 main.cpp)
