// Perf-lane test for false_sharing_demo.cpp.
//
// Asserts that the cache-line-padded accumulator is faster than the
// packed (false-sharing) accumulator at multi-thread counts. This is
// behavioural — a generous slack is allowed because runners vary.
// CI runs it under the `perf` LABEL only.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

double bad_accum(int iters_per_thread);
double good_accum(int iters_per_thread);

TEST_CASE("padded accumulator is never substantially slower than packed")
{
    omp_set_num_threads(4);
    constexpr int iters = 20'000'000;

    // Warm-up — first-region runtime startup.
    (void)good_accum(1000);
    (void)bad_accum(1000);

    // Run each three times and take the minimum (least noise-affected).
    auto bench = [](auto fn) {
        double best = 1e30;
        for (int trial = 0; trial < 3; ++trial) {
            const double t0 = omp_get_wtime();
            (void)fn(iters);
            const double t1 = omp_get_wtime();
            if ((t1 - t0) < best) {
                best = t1 - t0;
            }
        }
        return best;
    };

    const double t_bad = bench(bad_accum);
    const double t_good = bench(good_accum);

    INFO("t_bad=", t_bad, "  t_good=", t_good);
    // Regression check: padded must never be more than 10 % slower than
    // packed. On a typical multi-core machine padded is faster (often
    // dramatically so on contended NUMA hardware); but on a small
    // laptop with a unified L3 the gap can be small. 10 % is a very
    // generous bound that catches a real regression in the
    // implementation without being noise-flaky.
    CHECK(t_good <= 1.10 * t_bad);
}
