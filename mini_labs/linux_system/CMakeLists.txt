cmake_minimum_required(VERSION 3.5)
project(system_lab)
set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.46.1 REQUIRED COMPONENTS filesystem system)

add_executable(system_lab main.cpp
                          utils.cpp)

target_link_libraries(system_lab
        PRIVATE
        Boost::filesystem
        )
