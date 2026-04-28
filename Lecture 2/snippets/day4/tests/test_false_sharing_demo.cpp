#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

double bad_accum(int iters_per_thread);
double good_accum(int iters_per_thread);

TEST_CASE("false-sharing and padded variants produce identical results")
{
    constexpr int ITERS = 10'000;

    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        // Both variants should produce exactly `p * ITERS`.
        CHECK(bad_accum(ITERS) == doctest::Approx(static_cast<double>(p) * ITERS));
        CHECK(good_accum(ITERS) == doctest::Approx(static_cast<double>(p) * ITERS));
    }
}
