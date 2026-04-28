#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>
#include <vector>

void init_serial(std::vector<double>& v);
void init_first_touch(std::vector<double>& v);
double sum_parallel(const std::vector<double>& v);

TEST_CASE("serial and first-touch inits produce identical contents")
{
    constexpr std::size_t N = 1 << 14;

    std::vector<double> a(N);
    std::vector<double> b(N);

    omp_set_num_threads(4);
    init_serial(a);
    init_first_touch(b);

    for (std::size_t i = 0; i < N; ++i) {
        CHECK(a[i] == b[i]);
    }

    // Sum should also be identical.
    const double expected = static_cast<double>(N) * (N - 1) / 2.0;
    CHECK(sum_parallel(a) == doctest::Approx(expected));
    CHECK(sum_parallel(b) == doctest::Approx(expected));
}
