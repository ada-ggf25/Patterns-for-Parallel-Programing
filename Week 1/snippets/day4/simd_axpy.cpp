#include <cstddef>
#include <omp.h>

// SAXPY / DAXPY with `#pragma omp simd`.
//
// The `simd` directive asserts to the compiler that the loop iterations
// are independent and can be executed in parallel SIMD lanes — `x[i]`
// and `y[i]` read distinct elements and `r[i]` writes a distinct
// element per iteration. The compiler emits unaligned vector loads /
// stores, which on Zen 2 (and any post-Haswell Intel) cost essentially
// the same as aligned ones — alignment tweaks are out of scope here.

// snippet-begin: simd_only
void axpy_simd(double alpha, const double* x, const double* y, double* r, std::size_t n)
{
#pragma omp simd
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
#pragma omp parallel for simd default(none) firstprivate(alpha, x, y, r, n)
    for (std::size_t i = 0; i < n; ++i) {
        r[i] = (alpha * x[i]) + y[i];
    }
}
// snippet-end: parallel_simd
