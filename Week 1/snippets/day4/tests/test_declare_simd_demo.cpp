#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cmath>
#include <omp.h>
#include <vector>

void apply_smooth_step(std::vector<double>& v);
void apply_smooth_step_threaded(std::vector<double>& v);

TEST_CASE("apply_smooth_step at boundaries 0 and 1")
{
    std::vector<double> v{0.0, 1.0};
    apply_smooth_step(v);
    // smooth_step(0) = (1 - cos(0)) / 2 = 0
    // smooth_step(1) = (1 - cos(π)) / 2 = 1
    CHECK(v[0] == doctest::Approx(0.0).epsilon(1e-9));
    CHECK(v[1] == doctest::Approx(1.0).epsilon(1e-9));
}

TEST_CASE("threaded variant matches the scalar simd variant")
{
    constexpr std::size_t n = 4096;
    std::vector<double> a(n);
    std::vector<double> b(n);
    for (std::size_t i = 0; i < n; ++i) {
        a[i] = b[i] = 0.001 * static_cast<double>(i);
    }
    omp_set_num_threads(1);
    apply_smooth_step(a);
    omp_set_num_threads(4);
    apply_smooth_step_threaded(b);
    for (std::size_t i = 0; i < n; ++i) {
        CHECK(a[i] == doctest::Approx(b[i]).epsilon(1e-12));
    }
}
