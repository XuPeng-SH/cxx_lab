cmake_minimum_required(VERSION 3.5)
project(ADTLab)
set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.46.1 REQUIRED COMPONENTS filesystem system)

add_executable(ADTLab main.cpp)

target_link_libraries(ADTLab
        PRIVATE
        Boost::filesystem
        )
