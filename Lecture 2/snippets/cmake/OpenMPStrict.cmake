# Enforce OpenMP 5.1 at the compile level (for Clang).
#
# `-fopenmp-version=` is a Clang-only flag. GCC's supported OpenMP level
# tracks the compiler version itself; GCC 12+ covers OpenMP 5.1-level
# features without needing an explicit flag.
#
# Apple Clang's stock libomp also works via Homebrew; the find_package(OpenMP)
# call in the root CMakeLists locates it. This module appends the flag to
# whichever OpenMP target the find_package call exposes.

if(NOT TARGET OpenMP::OpenMP_CXX)
  message(FATAL_ERROR "OpenMPStrict: find_package(OpenMP) must run first")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(OpenMP::OpenMP_CXX INTERFACE -fopenmp-version=51)
else()
  message(STATUS "OpenMPStrict: non-Clang compiler (${CMAKE_CXX_COMPILER_ID}); "
                 "relying on compiler's default OpenMP level.")
endif()
