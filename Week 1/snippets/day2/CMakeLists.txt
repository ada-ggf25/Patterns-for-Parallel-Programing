# Day 2 — parallel regions, reductions, schedules.
#
# Each snippet .cpp is compiled with its own test harness (tests/test_<name>.cpp)
# against doctest. The same .cpp is also pasted verbatim into slides/day2.qmd via
# Quarto's include shortcode, so slides and tests cannot drift.

function(ppp_add_snippet_test name)
  add_executable(test_day2_${name}
    tests/test_${name}.cpp
    ${name}.cpp
  )
  # Order matters: ppp_warnings' -Wpedantic must appear before doctest's
  # -Wno-c2y-extensions on the compile line so the later silence wins.
  target_link_libraries(test_day2_${name}
    PRIVATE ppp_warnings OpenMP::OpenMP_CXX doctest
  )
  add_test(NAME day2_${name} COMMAND test_day2_${name})
  set_tests_properties(day2_${name} PROPERTIES LABELS "day2")
endfunction()

# Race-demo variant: uses the same snippet .cpp + a TSan-only test file
# (test_<name>_tsan.cpp). Labelled RACE_DEMO; ctest expects the TSan
# diagnostic in stdout/stderr — passes only when TSan flags the race.
function(ppp_add_race_demo_test name)
  add_executable(test_day2_${name}_tsan
    tests/test_${name}_tsan.cpp
    ${name}.cpp
  )
  target_link_libraries(test_day2_${name}_tsan
    PRIVATE ppp_warnings OpenMP::OpenMP_CXX doctest
  )
  add_test(NAME day2_${name}_tsan COMMAND test_day2_${name}_tsan)
  # Force TSan to NOT abort when it detects the (intended) race. CTest
  # checks PASS_REGULAR_EXPRESSION against stdout+stderr; if the test
  # aborts via SIGABRT, on some platforms (notably macOS) ctest
  # bypasses regex evaluation. exitcode=0 lets the test finish normally
  # while TSan still reports the race in stderr — which is what the
  # regex matches on.
  set_tests_properties(day2_${name}_tsan PROPERTIES
    LABELS "day2;RACE_DEMO"
    ENVIRONMENT "TSAN_OPTIONS=exitcode=0:halt_on_error=0"
    PASS_REGULAR_EXPRESSION "ThreadSanitizer: data race"
  )
endfunction()

ppp_add_snippet_test(parallel_region_hello)
ppp_add_snippet_test(reduction_sum)
ppp_add_snippet_test(schedule_comparison)
ppp_add_snippet_test(race_missing_default_none)
ppp_add_snippet_test(firstprivate_pitfall)
ppp_add_snippet_test(udr_user_defined_reduction)
ppp_add_snippet_test(spike_workload)

ppp_add_race_demo_test(race_missing_default_none)

# Standalone demo with its own main() — students compile + run directly
# (see day2 slides). No doctest, no test wiring; just a build target so
# the snippet can't rot. Optional ctest entry below in the RACE_DEMO
# label runs it under TSan to confirm the diagnostic still fires.
add_executable(race_demo_standalone race_demo_standalone.cpp)
target_link_libraries(race_demo_standalone
  PRIVATE ppp_warnings OpenMP::OpenMP_CXX
)
add_test(NAME day2_race_demo_standalone COMMAND race_demo_standalone)
set_tests_properties(day2_race_demo_standalone PROPERTIES
  LABELS "day2;RACE_DEMO"
  ENVIRONMENT "TSAN_OPTIONS=exitcode=0:halt_on_error=0"
  PASS_REGULAR_EXPRESSION "ThreadSanitizer: data race"
)
