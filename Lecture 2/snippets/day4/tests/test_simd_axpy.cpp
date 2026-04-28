#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <cstdlib>
#include <omp.h>

void axpy_simd(double alpha, const double* x, const double* y, double* r, std::size_t n);
void axpy_parallel_simd(double alpha, const double* x, const double* y, double* r,
                        std::size_t n);

namespace {

// Allocate a 64-byte aligned buffer; caller owns and must `std::free`.
double* aligned_alloc_64(std::size_t n)
{
    void* p = nullptr;
    if (posix_memalign(&p, 64, n * sizeof(double)) != 0) {
        return nullptr;
    }
    return static_cast<double*>(p);
}

}  // namespace

TEST_CASE("axpy_simd and axpy_parallel_simd produce the same result")
{
    constexpr std::size_t N = 4096;
    constexpr double alpha = 2.5;

    double* x = aligned_alloc_64(N);
    double* y = aligned_alloc_64(N);
    double* r_simd = aligned_alloc_64(N);
    double* r_par = aligned_alloc_64(N);

    REQUIRE(x != nullptr);
    REQUIRE(y != nullptr);
    REQUIRE(r_simd != nullptr);
    REQUIRE(r_par != nullptr);

    for (std::size_t i = 0; i < N; ++i) {
        x[i] = static_cast<double>(i);
        y[i] = static_cast<double>(i) * 0.5;
    }

    omp_set_num_threads(4);
    axpy_simd(alpha, x, y, r_simd, N);
    axpy_parallel_simd(alpha, x, y, r_par, N);

    for (std::size_t i = 0; i < N; ++i) {
        const double expected = (alpha * x[i]) + y[i];
        CHECK(r_simd[i] == doctest::Approx(expected));
        CHECK(r_par[i] == doctest::Approx(expected));
    }

    std::free(x);
    std::free(y);
    std::free(r_simd);
    std::free(r_par);
}
