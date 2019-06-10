cmake_minimum_required(VERSION 3.5)
project(kafka_lab)
set(CMAKE_CXX_STANDARD 11)

find_package(Boost 1.46.1 REQUIRED COMPONENTS filesystem system)
include_directories(/usr/local/include/librdkafka)


aux_source_directory(./ srcs)
link_directories(/usr/local/lib)

add_executable(kafka_lab ${srcs})

target_link_libraries(kafka_lab
        Boost::filesystem
        rdkafka++
        rdkafka
        )
