#include <cstddef>
#include <cstdlib>
#include <omp.h>

// Demonstrates NUMA first-touch policy: a page is placed on the NUMA node
// of the thread that *first writes* to it, not the thread that allocates.
//
// `std::vector<T>(N)` value-initialises every element — that loop runs on
// the constructing thread (master), counts as the first touch, and pre-empts
// any later parallel-init. To exercise first-touch correctly we allocate
// *uninitialised* memory via `posix_memalign` and let a parallel-for do
// the first write.
//
// 64-byte alignment is the universal answer on Rome: cache-line size +
// covers AVX2's 32-byte vector width + doubles as a false-sharing
// prevention hint. Same number used by `posix_memalign` callers across
// the day-4 SIMD cluster.

// snippet-begin: allocate
// Allocate `n` doubles aligned to 64 bytes. Returns *uninitialised* memory
// — no pages are mapped to any NUMA node until something first-writes.
// Caller owns and must `std::free`.
double* aligned_alloc_double(std::size_t n)
{
    void* raw = nullptr;
    if (posix_memalign(&raw, 64, n * sizeof(double)) != 0) {
        return nullptr;
    }
    return static_cast<double*>(raw);
}
// snippet-end: allocate

// snippet-begin: parallel_init
// Each thread first-touches the slice it will later read/write.
// Pages distribute across the team's NUMA domains automatically.
void init_first_touch(double* u, std::size_t n)
{
#pragma omp parallel for default(none) shared(u, n)
    for (std::size_t i = 0; i < n; ++i) {
        u[i] = static_cast<double>(i);
    }
}
// snippet-end: parallel_init

// snippet-begin: consumer
// Match the init's traversal pattern; each thread reads the slice it touched.
double sum_parallel(const double* u, std::size_t n)
{
    double s = 0.0;
#pragma omp parallel for default(none) shared(u, n) reduction(+ : s)
    for (std::size_t i = 0; i < n; ++i) {
        s += u[i];
    }
    return s;
}
// snippet-end: consumer
