// `omp_get_wtime()` returns wall-clock seconds since some fixed
// (implementation-defined) origin. The right way to time a parallel
// region is *outside* the region, with a single read on entry and
// exit.
//
// Pitfalls:
//
//   - Reading wtime *inside* a parallel region from every thread
//     gives you N timestamps, not a region duration.
//   - The first call to a parallel region pays a one-time runtime
//     start-up cost. If that matters, do a warm-up region first.
//   - Wall-clock time includes everything: kernel work, scheduling,
//     contention, NUMA traffic. That is usually exactly what you
//     want for a perf measurement, but it makes hot-path
//     micro-benchmarks noisy.

#include <cstddef>
#include <omp.h>
#include <vector>

double time_kernel_seconds(std::vector<double>& v)
{
    const std::size_t n = v.size();

    // snippet-begin: time_kernel_warmup
    // Warm-up: avoid measuring the first-region runtime startup.
#pragma omp parallel default(none) shared(v, n)
    {
#pragma omp for
        for (std::size_t i = 0; i < n; ++i) {
            v[i] = 0.0;
        }
    }
    // snippet-end: time_kernel_warmup

    // snippet-begin: time_kernel_timed
    const double t0 = omp_get_wtime();
#pragma omp parallel default(none) shared(v, n)
    {
#pragma omp for
        for (std::size_t i = 0; i < n; ++i) {
            v[i] = (v[i] * 1.001) + 1.0;
        }
    }
    const double t1 = omp_get_wtime();

    return t1 - t0;
    // snippet-end: time_kernel_timed
}

// snippet-begin: compare_struct
// Pair both timings in a struct so the caller can compute ratio,
// speedup, or absolute diff as needed.
struct TimingPair {
    double fast_s;
    double slow_s;
};
// snippet-end: compare_struct

// snippet-begin: compare_function
TimingPair compare_two_implementations(std::size_t n)
{
    std::vector<double> a(n);
    std::vector<double> b(n);

    const double t_fast = time_kernel_seconds(a);
    const double t_slow = time_kernel_seconds(b);

    return {t_fast, t_slow};
}
// snippet-end: compare_function
