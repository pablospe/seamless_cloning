cmake_minimum_required(VERSION 2.8)
project( seamless_cloning )

find_package( OpenCV REQUIRED )
# find_package( OpenCV REQUIRED PATHS "/home/spe/projects/opencv_git/installed/" )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable( seamless_cloning
                cloning.cpp
                homography.cpp
                interactive.cpp
                main.cpp
                path.cpp
                quadrilateral.cpp
                seamless_cloning.cpp
                utils.cpp
)

target_link_libraries( seamless_cloning ${OpenCV_LIBS} )
