#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>
#include <vector>

struct Stats {
    long n;
    double sum;
    double sum_sq;
};

Stats parallel_stats(const std::vector<double>& v);

TEST_CASE("user-defined Stats reduction computes correct mean and variance")
{
    // {1.0, 2.0, ..., 100.0}: n=100, sum=5050, sum_sq=338350,
    // mean=50.5, var = sum_sq/n - mean^2 = 3383.5 - 2550.25 = 833.25.
    std::vector<double> v;
    v.reserve(100);
    for (int i = 1; i <= 100; ++i) {
        v.push_back(static_cast<double>(i));
    }

    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        Stats s = parallel_stats(v);
        CHECK(s.n == 100);
        CHECK(s.sum == doctest::Approx(5050.0));
        CHECK(s.sum_sq == doctest::Approx(338350.0));

        const double mean = s.sum / static_cast<double>(s.n);
        const double var  = s.sum_sq / static_cast<double>(s.n) - mean * mean;
        CHECK(mean == doctest::Approx(50.5));
        CHECK(var  == doctest::Approx(833.25));
    }
}
