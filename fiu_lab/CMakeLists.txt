cmake_minimum_required(VERSION 3.5)
project(FiuLab)
set(CMAKE_CXX_STANDARD 11)

# option(FIU_ENABLE "select to enable fiu" TRUE)

aux_source_directory(./ source_files)
aux_source_directory(./src fiu_src)

include_directories(src)
include_directories(/usr/include)
include_directories(/usr/local/cuda/include)
include_directories(/home/xupeng/github/libfiu/libfiu/)

link_directories(/usr/local/cuda/lib64
                 /usr/lib/gcc/x86_64-linux-gnu/5/
                 /home/xupeng/github/libfiu/libfiu)

set(src ${source_files} ${fiu_src})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC -DELPP_THREAD_SAFE -DFIU_ENABLE")

add_executable(fiu_lab ${src})

set(fiu_lab_libs
    gflags
    fiu
    )

target_link_libraries(fiu_lab ${fiu_lab_libs})
