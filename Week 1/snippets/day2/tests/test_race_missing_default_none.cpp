#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

long count_races(int iterations);
long count_safely(int iterations);

// In the release lane this test ONLY checks the safe variant. The race
// variant's correctness is intentionally undefined (a lost-update race
// can produce any total ≤ p × iterations); we don't make a numerical
// claim about it here. The TSan lane (Phase 3) is what actually proves
// the race exists; this lane just confirms the safe variant is correct.
TEST_CASE("count_safely returns p * iterations regardless of thread count")
{
    const int iters = 1000;
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const long got = count_safely(iters);
        CHECK(got == static_cast<long>(p) * iters);
    }
}

// Sanity-check that the racy version at least *runs* (does not crash
// or deadlock). We deliberately do NOT assert on its return value.
TEST_CASE("count_races completes without crashing")
{
    omp_set_num_threads(2);
    const long got = count_races(100);
    CHECK(got >= 0);  // weakest possible postcondition; any total ok
}
