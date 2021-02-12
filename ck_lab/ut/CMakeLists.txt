cmake_minimum_required(VERSION 3.12)

project(myut)
set(THREADS_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} tests.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

enable_testing()

target_link_libraries(${PROJECT_NAME}
    gtest
    gtest_main
    stdc++fs
    Threads::Threads
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    /usr/local/include/gtest
    ${PROJECT_SOURCE_DIR}/..
)
