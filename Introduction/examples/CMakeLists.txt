cmake_minimum_required(VERSION 3.16)
project(ppp_intro_examples CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(serial)

find_package(OpenMP REQUIRED COMPONENTS CXX)
add_subdirectory(openmp)

find_package(MPI REQUIRED COMPONENTS CXX)
add_subdirectory(mpi)
