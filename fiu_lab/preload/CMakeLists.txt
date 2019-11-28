cmake_minimum_required(VERSION 3.5)

project(PreloadLib)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)

# include_directories(/usr/include)
# link_directories(/usr/lib/gcc/x86_64-linux-gnu/5/)

set(src ${source_files})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -O0 -g -Wall -fPIC -D_GNU_SOURCE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC")

add_library(preloadlib SHARED ${src})

set(libs
    dl
    )

target_link_libraries(preloadlib ${libs})
