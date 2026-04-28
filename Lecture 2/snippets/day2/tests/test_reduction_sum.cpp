#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <numeric>
#include <omp.h>
#include <vector>

double reduction_sum(const std::vector<double>& a);

TEST_CASE("reduction_sum matches std::accumulate within eps")
{
    std::vector<double> a(1024);
    for (std::size_t i = 0; i < a.size(); ++i)
        a[i] = 1.0 + 0.001 * static_cast<double>(i);

    const double expected = std::accumulate(a.begin(), a.end(), 0.0);

    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const double got = reduction_sum(a);
        CHECK(got == doctest::Approx(expected).epsilon(1e-10));
    }
}
