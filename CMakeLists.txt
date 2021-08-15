cmake_minimum_required(VERSION 3.13)
project(pipelines_cpp)
set(CMAKE_CXX_STANDARD 14)

# Add main.cpp file of project root directory as source file
set(GLOB SOURCE_FILES *.cpp *.h)

find_package (Threads)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(pipelines_cpp main.cpp)

target_link_libraries (pipelines_cpp ${CMAKE_THREAD_LIBS_INIT})