cmake_minimum_required(VERSION 3.5)
project(MetaProgramming)

add_subdirectory(3rd_party/google-test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DV2" CACHE STRING "Enable V2 Info" FORCE)
message("${CMAKE_CXX_FLAGS}")

set(LIBSOURCES src/container.cpp
    src/my_templates_demo.cpp)

add_library(MetaProgrammingStaticLibrary STATIC ${LIBSOURCES})
target_compile_features(MetaProgrammingStaticLibrary PRIVATE cxx_std_11)
target_include_directories(MetaProgrammingStaticLibrary
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include)

add_library(MetaProgrammingDynamicLibrary SHARED ${LIBSOURCES})
target_compile_features(MetaProgrammingDynamicLibrary PRIVATE cxx_std_11)
target_include_directories(MetaProgrammingDynamicLibrary
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include)


set(EXESOURCES src/main.cpp)

add_executable(StaticTesting ${EXESOURCES})

target_compile_features(StaticTesting PRIVATE cxx_std_11)

target_compile_definitions(StaticTesting PRIVATE V3)

target_link_libraries(StaticTesting
    PRIVATE
    MetaProgrammingStaticLibrary)

add_executable(SharedTesting ${EXESOURCES})

target_compile_features(SharedTesting PRIVATE cxx_std_11)

target_link_libraries(SharedTesting
    PRIVATE
    MetaProgrammingDynamicLibrary)

#############################################
############### INSTALL #####################
#############################################

install(TARGETS MetaProgrammingStaticLibrary
    DESTINATION lib)

install(TARGETS MetaProgrammingDynamicLibrary
    DESTINATION lib)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
    DESTINATION include)

enable_testing()
add_subdirectory(unittests)

add_test(NAME test_all
        COMMAND unittests/myTest)
