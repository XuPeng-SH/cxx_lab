cmake_minimum_required(VERSION 3.5)
project(TraceLab)
set(CMAKE_CXX_STANDARD 11)

aux_source_directory(./ source_files)

include_directories(/usr/include)
include_directories(/usr/local/include)

link_directories(/usr/lib/gcc/x86_64-linux-gnu/5/
    /usr/lib/x86_64-linux-gnu
    /usr/lib
    /usr/local/lib
    )

set(src ${source_files})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fPIC")

add_executable(trace_lab ${src})

set(lab_libs
    gflags
    opentracing
    opentracing_mocktracer
    protobuf
    pthread
    grpc
    grpc++
    )

target_link_libraries(trace_lab ${lab_libs})
