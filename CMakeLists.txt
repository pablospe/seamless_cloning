cmake_minimum_required(VERSION 2.8)
project( seamless_cloning )

find_package( OpenCV REQUIRED PATHS "/home/sppablo/wrk/opencv-2.4.6.1/installed/" )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( seamless_cloning
                cloning.cpp
                interactive.cpp
                main.cpp
                path.cpp
                seamless_cloning.cpp
                utils.cpp
)

target_link_libraries( seamless_cloning ${OpenCV_LIBS} )
