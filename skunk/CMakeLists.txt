cmake_minimum_required(VERSION 3.8)
project(Skunk)
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_CXX_STANDARD 20)

add_executable(Skunk main.cpp board.cpp board.h)

if(WIN32)
    target_link_libraries(Skunk wsock32 ws2_32)
endif()