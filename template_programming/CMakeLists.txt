cmake_minimum_required(VERSION 2.6)
project(MetaProgramming)
set(CMAKE_CXX_STANDARD 11)

add_executable(MetaProgramming main.cpp
                               container.cpp
                               my_templates_demo.cpp)
