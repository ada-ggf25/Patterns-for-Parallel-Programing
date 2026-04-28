#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

double sum_naive(int rows, int cols);
double sum_collapsed(int rows, int cols);

TEST_CASE("naive and collapsed sums match")
{
    constexpr int rows = 4;
    constexpr int cols = 1000;
    const double expected = static_cast<double>(rows) * cols;

    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        CHECK(sum_naive(rows, cols) == doctest::Approx(expected));
        CHECK(sum_collapsed(rows, cols) == doctest::Approx(expected));
    }
}
