cmake_minimum_required(VERSION 3.12)

project(myTest)

add_executable(${PROJECT_NAME} unittests.cpp)

enable_testing()

target_link_libraries(${PROJECT_NAME}
    MetaProgrammingStaticLibrary
    GTest::GTest
    GTest::Main
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include)
