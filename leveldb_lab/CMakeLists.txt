cmake_minimum_required(VERSION 3.12)
project(LeveldbLab)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)

include_directories(/usr/include)
include_directories(/usr/local/include
    /home/xupeng/github/leveldb/include)

link_directories(/usr/lib/gcc/x86_64-linux-gnu/5/
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
    /home/xupeng/github/leveldb/build
    )

set(src ${source_files})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -fPIC")

add_executable(leveldb_lab ${src})

set(lab_libs
    leveldb
    gflags
    pthread
    snappy
    )

target_link_libraries(leveldb_lab ${lab_libs})
