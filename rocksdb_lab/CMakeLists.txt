cmake_minimum_required(VERSION 3.12)
project(rocksdbLab)
set(CMAKE_CXX_STANDARD 14)

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

if(WITH_FAISS)
    message("WITH FAISS=${WITH_FAISS}")
    include_directories(/usr/local/cuda/include)
    include_directories(/home/xupeng/github/faiss/include)
    include_directories(/home/xupeng/github/faiss)

    link_directories(/usr/local/cuda/lib64)
endif()

set(src ${source_files})

if (WITH_FAISS)
    add_definitions(-DWITH_FAISS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC -DELPP_THREAD_SAFE -fopenmp")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC")
endif()
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g -fPIC")

add_executable(rocksdb_lab ${src})

if(WITH_FAISS)
    set(lab_libs
        rocksdb
        gflags
        pthread
        /home/xupeng/github/faiss/libfaiss.a
        gfortran
        cudart
        cublas
        openblas
        )
else()
    set(lab_libs
        rocksdb
        gflags
        pthread
        )
endif()

target_link_libraries(rocksdb_lab ${lab_libs})
