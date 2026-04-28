cmake_minimum_required(VERSION 3.20)
project(ppp-openmp-snippets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(OpenMP REQUIRED)
include(OpenMPStrict)

option(ENABLE_TSAN  "Enable ThreadSanitizer (race-demo tests depend on this)" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
if(ENABLE_TSAN)
  include(EnableTSan)
endif()
if(ENABLE_UBSAN)
  include(EnableUBSan)
endif()

# Shared warning profile — strict but teachable.
add_library(ppp_warnings INTERFACE)
target_compile_options(ppp_warnings INTERFACE
  -Wall -Wextra -Wpedantic -Wshadow
  -Wno-unused-parameter   # allowed in didactic examples
)

# doctest (single header vendored in third_party/).
# Exposed as a SYSTEM include so compiler warnings from macro expansions in the
# doctest header are suppressed (snippet .cpp files keep the strict profile).
# doctest's TEST_CASE macro uses __COUNTER__ which Clang 22+ flags under
# -Wpedantic; the suppressor must appear *after* the ppp_warnings flags on the
# command line. Callers must list doctest after ppp_warnings in
# target_link_libraries so CMake emits the interface flags in that order.
add_library(doctest INTERFACE)
target_include_directories(doctest SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
target_compile_options(doctest INTERFACE -Wno-c2y-extensions)

enable_testing()

# Per-day snippet collections registered below. Each subdirectory adds its own
# targets and ctest entries.
add_subdirectory(recap)
add_subdirectory(day2)
add_subdirectory(day3)
add_subdirectory(day4)
