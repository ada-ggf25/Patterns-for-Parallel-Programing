# Day 3 — synchronisation, memory model, tasks.

function(ppp_add_day3_test name)
  add_executable(test_day3_${name}
    tests/test_${name}.cpp
    ${name}.cpp
  )
  target_link_libraries(test_day3_${name}
    PRIVATE ppp_warnings OpenMP::OpenMP_CXX doctest
  )
  add_test(NAME day3_${name} COMMAND test_day3_${name})
  set_tests_properties(day3_${name} PROPERTIES LABELS "day3")
endfunction()

function(ppp_add_day3_race_demo_test name)
  add_executable(test_day3_${name}_tsan
    tests/test_${name}_tsan.cpp
    ${name}.cpp
  )
  target_link_libraries(test_day3_${name}_tsan
    PRIVATE ppp_warnings OpenMP::OpenMP_CXX doctest
  )
  add_test(NAME day3_${name}_tsan COMMAND test_day3_${name}_tsan)
  # See day2/CMakeLists.txt for why we override TSAN_OPTIONS here.
  set_tests_properties(day3_${name}_tsan PROPERTIES
    LABELS "day3;RACE_DEMO"
    ENVIRONMENT "TSAN_OPTIONS=exitcode=0:halt_on_error=0"
    PASS_REGULAR_EXPRESSION "ThreadSanitizer: data race"
  )
endfunction()

ppp_add_day3_test(critical_vs_atomic)
ppp_add_day3_test(tasks_basic)
ppp_add_day3_test(taskloop_apply)
ppp_add_day3_test(barriers_explicit)
ppp_add_day3_test(task_dependences)
ppp_add_day3_test(task_locks)
ppp_add_day3_test(single_master_nowait)
