#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <cstdlib>
#include <omp.h>

double* aligned_alloc_double(std::size_t n);
void init_first_touch(double* u, std::size_t n);
double sum_parallel(const double* u, std::size_t n);

TEST_CASE("posix_memalign + parallel first-touch + parallel sum produce expected total")
{
    constexpr std::size_t N = 1 << 14;

    omp_set_num_threads(4);
    double* u = aligned_alloc_double(N);
    REQUIRE(u != nullptr);

    init_first_touch(u, N);

    const double expected = static_cast<double>(N) * (N - 1) / 2.0;
    CHECK(sum_parallel(u, N) == doctest::Approx(expected));

    std::free(u);
}
