cmake_minimum_required(VERSION 3.15)
project(zaster_cmake_tests C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${ZASTER_ROOT} zaster_build)

# Test 1: C API with zstd
if(ZASTER_LINK_ZSTD)
    add_executable(test_c test_c.c)
    target_link_libraries(test_c PRIVATE zaster::zaster)

    # Test 2: C++ API with zstd
    add_executable(test_cpp test_cpp.cpp)
    target_link_libraries(test_cpp PRIVATE zaster::zaster)
endif()

# Test 3: C API without zstd (only if ZASTER_LINK_ZSTD is OFF)
if(NOT ZASTER_LINK_ZSTD)
    add_executable(test_no_zstd test_no_zstd.c)
    target_link_libraries(test_no_zstd PRIVATE zaster::zaster)
endif()
