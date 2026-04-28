#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <omp.h>
#include <vector>

void apply_taskloop(std::vector<double>& v, int grainsize);
void apply_for(std::vector<double>& v);

TEST_CASE("taskloop and parallel-for produce identical results")
{
    constexpr std::size_t N = 1024;

    std::vector<double> a(N);
    std::vector<double> b(N);
    for (std::size_t i = 0; i < N; ++i) {
        a[i] = b[i] = static_cast<double>(i) * 0.1;
    }

    omp_set_num_threads(4);
    apply_for(a);
    apply_taskloop(b, 32);

    for (std::size_t i = 0; i < N; ++i) {
        CHECK(a[i] == doctest::Approx(b[i]));
    }
}
