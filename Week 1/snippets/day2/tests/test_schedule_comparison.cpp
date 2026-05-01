#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <omp.h>

double sum_static(std::size_t n);
double sum_dynamic_64(std::size_t n);
double sum_guided(std::size_t n);

TEST_CASE("all three schedules produce the same numerical result")
{
    constexpr std::size_t N = 2048;
    omp_set_num_threads(4);

    const double s = sum_static(N);
    const double d = sum_dynamic_64(N);
    const double g = sum_guided(N);

    // Schedule affects *order* of floating-point additions; results are
    // identical only up to rounding. 1e-8 relative tolerance is generous
    // without hiding real correctness bugs.
    CHECK(s == doctest::Approx(d).epsilon(1e-8));
    CHECK(s == doctest::Approx(g).epsilon(1e-8));
}
