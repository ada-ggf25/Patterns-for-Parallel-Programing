#include <cstddef>
#include <omp.h>

// SAXPY / DAXPY with `#pragma omp simd`.
//
// `a`, `b`, `c` are assumed aligned to at least 64 bytes by the caller.
// `safelen(8)` asserts that at least 8 consecutive iterations have no
// cross-iteration data dependence — safe here because `x[i]` and `y[i]`
// read distinct elements and `r[i]` writes a distinct element per iter.

// snippet-begin: simd_only
void axpy_simd(double alpha, const double* x, const double* y, double* r, std::size_t n)
{
#pragma omp simd aligned(x, y, r : 64) safelen(8)
    for (std::size_t i = 0; i < n; ++i) {
        r[i] = (alpha * x[i]) + y[i];
    }
}
// snippet-end: simd_only

// Threaded+SIMD composition. The `parallel for` distributes loop iterations
// across threads; `simd` vectorises within each chunk. `collapse` would be
// used for nested loops; not needed here.
// snippet-begin: parallel_simd
void axpy_parallel_simd(double alpha, const double* x, const double* y, double* r, std::size_t n)
{
#pragma omp parallel for simd aligned(x, y, r : 64) safelen(8) default(none)          \
    firstprivate(alpha, x, y, r, n)
    for (std::size_t i = 0; i < n; ++i) {
        r[i] = (alpha * x[i]) + y[i];
    }
}
// snippet-end: parallel_simd
