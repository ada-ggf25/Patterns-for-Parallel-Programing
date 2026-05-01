#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <omp.h>
#include <vector>

void axpy_simd(double alpha, const double* x, const double* y, double* r, std::size_t n);
void axpy_parallel_simd(double alpha, const double* x, const double* y, double* r,
                        std::size_t n);

TEST_CASE("axpy_simd and axpy_parallel_simd produce the same result")
{
    constexpr std::size_t N = 4096;
    constexpr double alpha = 2.5;

    std::vector<double> x(N);
    std::vector<double> y(N);
    std::vector<double> r_simd(N);
    std::vector<double> r_par(N);

    for (std::size_t i = 0; i < N; ++i) {
        x[i] = static_cast<double>(i);
        y[i] = static_cast<double>(i) * 0.5;
    }

    omp_set_num_threads(4);
    axpy_simd(alpha, x.data(), y.data(), r_simd.data(), N);
    axpy_parallel_simd(alpha, x.data(), y.data(), r_par.data(), N);

    for (std::size_t i = 0; i < N; ++i) {
        const double expected = (alpha * x[i]) + y[i];
        CHECK(r_simd[i] == doctest::Approx(expected));
        CHECK(r_par[i] == doctest::Approx(expected));
    }
}
