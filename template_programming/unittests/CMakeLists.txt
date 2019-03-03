cmake_minimum_required(VERSION 3.12)

project(myTest)

set(THREADS_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} unittests.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

enable_testing()


target_link_libraries(${PROJECT_NAME}
    MetaProgrammingStaticLibrary
    gtest
    gtest_main
    Threads::Threads # pthread is OK
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    /usr/local/include/gtest
    ${PROJECT_SOURCE_DIR}/include)
