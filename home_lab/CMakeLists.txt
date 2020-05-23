cmake_minimum_required(VERSION 3.5)
project(HomeLab)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)

include_directories(src)
include_directories(/usr/include)

# link_directories(/usr/local/cuda/lib64
#                  /usr/lib/gcc/x86_64-linux-gnu/5/)

set(src ${source_files})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC")

add_executable(home_lab ${src})

set(home_lab_libs
    gflags
    )

target_link_libraries(home_lab ${home_lab_libs})
