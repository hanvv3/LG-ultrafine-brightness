cmake_minimum_required(VERSION 3.13)
project(LG_ultrafine_brightness)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS -g)

set(libusb_1_FIND_REQUIRED TRUE)
include(${PROJECT_SOURCE_DIR}/cmake/Findlibusb-1.0.cmake)
include_directories(${LIBUSB_1_INCLUDE_DIRS})

add_executable(ufctl main.cpp UltrafineDisplay.cpp CLIHandler.cpp Globals.h UltrafineDisplay.h CLIHandler.h)
target_link_libraries(ufctl ${LIBUSB_1_LIBRARIES} ncurses)
