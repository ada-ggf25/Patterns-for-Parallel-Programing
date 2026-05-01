cmake_minimum_required(VERSION 3.20)
project(PPPAssessment1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenMP REQUIRED)

# Shared warning profile for all student targets.
add_library(ppp_warnings INTERFACE)
target_compile_options(ppp_warnings INTERFACE
  -Wall -Wextra -Wpedantic
  -Wno-unused-parameter
)

# Pin OpenMP 5.1 at compile time for Clang. GCC does NOT accept
# -fopenmp-version= (its supported OpenMP level tracks the compiler version
# itself; GCC 12+ already covers 5.1-level features).
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(OpenMP::OpenMP_CXX INTERFACE -fopenmp-version=51)
endif()

# A1 — numerical integration
add_executable(integrate
  integrate.cpp
  integrate_serial.cpp
)
target_include_directories(integrate PRIVATE .)
target_link_libraries(integrate PRIVATE OpenMP::OpenMP_CXX ppp_warnings)
