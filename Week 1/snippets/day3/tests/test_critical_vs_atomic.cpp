#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <numeric>
#include <omp.h>
#include <vector>

long long counter_critical(const std::vector<int>& v);
long long counter_atomic(const std::vector<int>& v);
long long counter_reduction(const std::vector<int>& v);

TEST_CASE("three accumulation strategies agree numerically")
{
    std::vector<int> v(4096);
    for (std::size_t i = 0; i < v.size(); ++i) v[i] = static_cast<int>(i % 7);

    const long long expected = std::accumulate(v.begin(), v.end(), 0LL);

    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        CHECK(counter_critical(v) == expected);
        CHECK(counter_atomic(v) == expected);
        CHECK(counter_reduction(v) == expected);
    }
}
