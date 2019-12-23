cmake_minimum_required(VERSION 3.5)
project(FutextLab)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)
aux_source_directory(./src src)

include_directories(src)
include_directories(/usr/include)
include_directories(/usr/local/cuda/include)

link_directories(/usr/local/cuda/lib64
                 /usr/lib/gcc/x86_64-linux-gnu/5/)

set(src ${source_files} ${src})

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -fPIC -DELPP_THREAD_SAFE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC -DELPP_THREAD_SAFE")

add_executable(futex_lab ${src})

set(futex_lab_libs
    pthread
    # gflags
    )

target_link_libraries(futex_lab ${futex_lab_libs})
