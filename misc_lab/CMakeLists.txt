cmake_minimum_required(VERSION 3.5)
project(MiscLab)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)
aux_source_directory(./src faiss_src)

include_directories(src)
include_directories(/usr/include)
include_directories(/usr/local/cuda/include)

link_directories(/usr/local/cuda/lib64
                 /usr/lib/gcc/x86_64-linux-gnu/5/)

set(src ${source_files} ${faiss_src})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC -DELPP_THREAD_SAFE")

add_executable(misc_lab ${src})

set(misc_lab_libs
    gflags
    )

target_link_libraries(misc_lab ${misc_lab_libs})
