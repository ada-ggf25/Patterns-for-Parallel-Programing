// TSan-lane test for race_missing_default_none.cpp.
//
// Built with -fsanitize=thread + Archer/OMPT. Exercises the racy
// variant deliberately. TSan halts on the race; CTest's
// PASS_REGULAR_EXPRESSION on the test entry matches the TSan
// diagnostic, so a halted process counts as a passing race-demo run.
//
// Excluded from the release lane via the RACE_DEMO ctest LABEL.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

long count_races(int iterations);

TEST_CASE("count_races provokes the race for ThreadSanitizer to flag")
{
    omp_set_num_threads(4);
    // Many iterations so TSan's sampling reliably catches it.
    (void)count_races(10'000);
    // No assertion — under TSan this exits via halt_on_error=1; under
    // release the test would *not* be run (excluded by LABEL).
}
