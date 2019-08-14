cmake_minimum_required(VERSION 3.5)
project(FaisLab)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)
aux_source_directory(./src faiss_src)

include_directories(src)
include_directories(/usr/include)
include_directories(/usr/local/cuda/include)
include_directories(/home/xupeng/github/faiss/include)
include_directories(/home/xupeng/github/faiss/)
# include_directories(/home/xupeng/github/milvus/cpp/cmake_build/faiss_ep-prefix/src/faiss_ep/include)

link_directories(/usr/local/cuda/lib64
                 /usr/lib/gcc/x86_64-linux-gnu/5/)

set(src ${source_files} ${faiss_src})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC -DELPP_THREAD_SAFE -fopenmp")

add_executable(faiss_lab ${src})

set(faiss_lab_libs
    /home/xupeng/github/faiss/libfaiss.a
    # /home/xupeng/gitlab/milvus/cpp/cmake_build/faiss_ep-prefix/src/faiss_ep/lib/libfaiss.a
    # /home/xupeng/gitlab/milvus/cpp/cmake_build/lapack_ep-prefix/src/lapack_ep/lib/liblapack.a
    # /home/xupeng/gitlab/milvus/cpp/cmake_build/openblas_ep-prefix/src/openblas_ep/libopenblas.a
    # /home/xupeng/github/milvus/cpp/cmake_build/faiss_ep-prefix/src/faiss_ep/lib/libfaiss.a
    # /home/xupeng/github/milvus/cpp/cmake_build/lapack_ep-prefix/src/lapack_ep/lib/liblapack.a
    # /home/xupeng/github/milvus/cpp/cmake_build/openblas_ep-prefix/src/openblas_ep/libopenblas.a
    gflags
    gfortran
    cudart
    cublas
    openblas
    )

target_link_libraries(faiss_lab ${faiss_lab_libs})
