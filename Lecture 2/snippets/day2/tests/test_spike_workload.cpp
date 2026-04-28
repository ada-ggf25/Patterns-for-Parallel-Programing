#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <omp.h>

double time_schedule_static(std::size_t n, double& out_sum);
double time_schedule_dynamic(std::size_t n, double& out_sum, int chunk);
double time_schedule_guided(std::size_t n, double& out_sum);

TEST_CASE("all three schedules produce numerically identical results")
{
    constexpr std::size_t n = 4000;
    omp_set_num_threads(4);

    double sum_s = 0.0;
    double sum_d = 0.0;
    double sum_g = 0.0;
    (void)time_schedule_static(n, sum_s);
    (void)time_schedule_dynamic(n, sum_d, /*chunk=*/64);
    (void)time_schedule_guided(n, sum_g);

    // Floating-point reduction order can differ; tolerate a relative
    // mismatch up to 1e-9.
    CHECK(sum_s == doctest::Approx(sum_d).epsilon(1e-9));
    CHECK(sum_s == doctest::Approx(sum_g).epsilon(1e-9));
}

TEST_CASE("each schedule returns a non-negative duration")
{
    constexpr std::size_t n = 1000;
    double sum = 0.0;
    omp_set_num_threads(2);
    CHECK(time_schedule_static(n, sum) >= 0.0);
    CHECK(time_schedule_dynamic(n, sum, 64) >= 0.0);
    CHECK(time_schedule_guided(n, sum) >= 0.0);
}
