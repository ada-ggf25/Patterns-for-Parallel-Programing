# ThreadSanitizer profile for snippet + race-demo tests.
#
# Requires Clang + LLVM libomp. GCC's libgomp has limited TSan instrumentation
# and produces false negatives on some OpenMP races.

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(WARNING
    "EnableTSan: CXX compiler is ${CMAKE_CXX_COMPILER_ID}. "
    "Race-demo tests may not trigger TSan reports under GCC. "
    "Prefer Clang + LLVM libomp for TSan runs.")
endif()

add_compile_options(-fsanitize=thread -g -O1 -fno-omit-frame-pointer)
add_link_options(-fsanitize=thread)

message(STATUS "TSan enabled: -fsanitize=thread -g -O1")
