#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <omp.h>
#include <vector>

double time_kernel_seconds(std::vector<double>& v);

struct TimingPair {
    double fast_s;
    double slow_s;
};
TimingPair compare_two_implementations(std::size_t n);

TEST_CASE("time_kernel_seconds returns a non-negative duration")
{
    std::vector<double> v(8192);
    omp_set_num_threads(2);
    const double dt = time_kernel_seconds(v);
    CHECK(dt >= 0.0);
}

TEST_CASE("compare_two_implementations returns plausible numbers")
{
    omp_set_num_threads(2);
    const auto p = compare_two_implementations(4096);
    CHECK(p.fast_s >= 0.0);
    CHECK(p.slow_s >= 0.0);
}
