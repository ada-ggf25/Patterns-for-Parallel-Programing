#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>
#include <vector>

void produce_then_consume(std::vector<double>& a, std::vector<double>& b);
void independent_fills(std::vector<double>& a, std::vector<double>& b);
int explicit_barrier_demo();

TEST_CASE("produce_then_consume sees correct values across threads")
{
    constexpr std::size_t n = 1024;
    std::vector<double> a(n);
    std::vector<double> b(n);
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        produce_then_consume(a, b);
        for (std::size_t i = 0; i < n; ++i) {
            CHECK(b[i] == doctest::Approx(2.0 * static_cast<double>(i)));
        }
    }
}

TEST_CASE("independent_fills produces correct constants under nowait")
{
    constexpr std::size_t n = 2048;
    std::vector<double> a(n);
    std::vector<double> b(n);
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        independent_fills(a, b);
        for (std::size_t i = 0; i < n; ++i) {
            CHECK(a[i] == doctest::Approx(1.0));
            CHECK(b[i] == doctest::Approx(2.0));
        }
    }
}

TEST_CASE("explicit_barrier_demo counts every thread twice")
{
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const int total = explicit_barrier_demo();
        CHECK(total == 2 * p);
    }
}
