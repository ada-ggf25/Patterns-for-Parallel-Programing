// Spike-workload schedule-sweep harness.
//
// Same kernel as schedule_comparison.cpp but the timing is exposed
// directly so the slide on schedule-sweep methodology can show the
// idiom: pick a schedule by *measuring*, not by guessing.
//
// `cost(i)` returns "expensive" for one in every ten iterations and
// "cheap" otherwise — a spike workload that mirrors A1's `f(x)`
// integrand.

#include <cstddef>
#include <omp.h>

namespace {
inline int cost(std::size_t i)
{
    return (i % 10 == 0) ? 2000 : 50;
}

inline double busy_work(int iters)
{
    double s = 0.0;
    for (int k = 0; k < iters; ++k) {
        s += 1.0 / (1.0 + (0.001 * static_cast<double>(k)));
    }
    return s;
}
}  // namespace

// snippet-begin: harness
// Time one schedule across `n` iterations. Returns the wall-clock
// duration in seconds. Sum is computed but discarded — the test asserts
// numerical equivalence with the other variants.
double time_schedule_static(std::size_t n, double& out_sum)
{
    double sum = 0.0;
    const double t0 = omp_get_wtime();
#pragma omp parallel for default(none) shared(n) reduction(+ : sum) schedule(static)
    for (std::size_t i = 0; i < n; ++i) {
        sum += busy_work(cost(i));
    }
    out_sum = sum;
    return omp_get_wtime() - t0;
}

double time_schedule_dynamic(std::size_t n, double& out_sum, int chunk)
{
    double sum = 0.0;
    const double t0 = omp_get_wtime();
#pragma omp parallel for default(none) shared(n) firstprivate(chunk)                       \
    reduction(+ : sum) schedule(dynamic, 64)
    for (std::size_t i = 0; i < n; ++i) {
        sum += busy_work(cost(i));
    }
    out_sum = sum;
    return omp_get_wtime() - t0;
}

double time_schedule_guided(std::size_t n, double& out_sum)
{
    double sum = 0.0;
    const double t0 = omp_get_wtime();
#pragma omp parallel for default(none) shared(n) reduction(+ : sum) schedule(guided)
    for (std::size_t i = 0; i < n; ++i) {
        sum += busy_work(cost(i));
    }
    out_sum = sum;
    return omp_get_wtime() - t0;
}
// snippet-end: harness
