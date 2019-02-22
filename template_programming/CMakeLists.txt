cmake_minimum_required(VERSION 3.5)
project(MetaProgramming)

set(LIBSOURCES src/main.cpp)

add_library(MetaProgrammingLibrary STATIC ${LIBSOURCES})

target_compile_features(MetaProgrammingLibrary PRIVATE cxx_std_11)

target_include_directories(MetaProgrammingLibrary
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include)


set(EXESOURCES src/container.cpp
    src/my_templates_demo.cpp)

add_executable(Testing ${EXESOURCES})

target_compile_features(Testing PRIVATE cxx_std_11)

target_link_libraries(Testing
    PRIVATE
    MetaProgrammingLibrary)
