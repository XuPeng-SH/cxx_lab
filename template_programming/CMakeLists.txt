cmake_minimum_required(VERSION 3.5)
project(MetaProgramming)

set(SOURCES src/main.cpp
    src/container.cpp
    src/my_templates_demo.cpp)

add_executable(MetaProgramming ${SOURCES})

target_compile_features(MetaProgramming PRIVATE cxx_std_11)

target_include_directories(MetaProgramming
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include)
