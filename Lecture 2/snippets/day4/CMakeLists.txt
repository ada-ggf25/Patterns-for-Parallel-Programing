# Day 4 — performance, NUMA, SIMD.

function(ppp_add_day4_test name)
  add_executable(test_day4_${name}
    tests/test_${name}.cpp
    ${name}.cpp
  )
  target_link_libraries(test_day4_${name}
    PRIVATE ppp_warnings OpenMP::OpenMP_CXX doctest
  )
  add_test(NAME day4_${name} COMMAND test_day4_${name})
  set_tests_properties(day4_${name} PROPERTIES LABELS "day4")
endfunction()

# Perf-lane variant: same snippet, separate test_<name>_perf.cpp file.
# Asserts a behavioural delta (e.g. padded < unpadded). Labelled `perf`;
# CI runs only when the perf lane is enabled, since these are noisy on
# small / virtualised runners.
function(ppp_add_day4_perf_test name)
  add_executable(test_day4_${name}_perf
    tests/test_${name}_perf.cpp
    ${name}.cpp
  )
  target_link_libraries(test_day4_${name}_perf
    PRIVATE ppp_warnings OpenMP::OpenMP_CXX doctest
  )
  add_test(NAME day4_${name}_perf COMMAND test_day4_${name}_perf)
  set_tests_properties(day4_${name}_perf PROPERTIES LABELS "day4;perf")
endfunction()

ppp_add_day4_test(false_sharing_demo)
ppp_add_day4_test(numa_first_touch)
ppp_add_day4_test(simd_axpy)
ppp_add_day4_test(omp_get_wtime_timing)
ppp_add_day4_test(collapse_loop_transform)
ppp_add_day4_test(declare_simd_demo)

ppp_add_day4_perf_test(false_sharing_demo)
