cmake_minimum_required(VERSION 2.6)
project(MetaProgramming)

add_executable(MetaProgramming main.cpp
                               container.cpp
                               my_templates_demo.cpp)

target_compile_features(MetaProgramming PRIVATE cxx_std_11)
