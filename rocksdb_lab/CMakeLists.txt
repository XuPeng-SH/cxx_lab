cmake_minimum_required(VERSION 3.12)
project(rocksdbLab)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)

include_directories(/usr/include)
include_directories(/usr/local/include
    /home/xupeng/github/rocksdb/include)

link_directories(/usr/lib/gcc/x86_64-linux-gnu/5/
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
    /home/xupeng/github/rocksdb/build
    )

set(src ${source_files})
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fPIC")

add_executable(rocksdb_lab ${src})

set(lab_libs
    rocksdb
    gflags
    pthread
    )

target_link_libraries(rocksdb_lab ${lab_libs})
