#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>
#include <vector>

void simulate_with_seed(std::vector<double>& out, double seed);
double weighted_sum(const std::vector<double>& v, double weight);

TEST_CASE("simulate_with_seed produces deterministic per-iter results")
{
    constexpr std::size_t n = 1024;
    constexpr double seed = 0.5;
    std::vector<double> a(n);
    std::vector<double> b(n);

    omp_set_num_threads(1);
    simulate_with_seed(a, seed);
    omp_set_num_threads(4);
    simulate_with_seed(b, seed);

    for (std::size_t i = 0; i < n; ++i) {
        CHECK(a[i] == doctest::Approx(b[i]).epsilon(1e-12));
    }
}

TEST_CASE("weighted_sum matches the serial total")
{
    std::vector<double> v(2048);
    for (std::size_t i = 0; i < v.size(); ++i) {
        v[i] = static_cast<double>(i);
    }
    constexpr double w = 0.25;

    double serial = 0.0;
    for (double x : v) {
        serial += x * w;
    }

    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const double got = weighted_sum(v, w);
        CHECK(got == doctest::Approx(serial).epsilon(1e-9));
    }
}
