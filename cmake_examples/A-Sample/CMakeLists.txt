cmake_minimum_required(VERSION 3.5)

project(AExample)

set(SOURCES src/main.cpp
           src/hello.cpp)

add_executable(AExample ${SOURCES})

target_include_directories(AExample
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include)

target_compile_features(AExample PRIVATE cxx_std_11)
