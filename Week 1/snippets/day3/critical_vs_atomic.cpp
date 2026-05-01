#include <cstddef>
#include <omp.h>
#include <vector>

// Demonstrates three ways to accumulate concurrently into a single scalar:
//
//   counter_critical  — #pragma omp critical      (slow, general-purpose)
//   counter_atomic    — #pragma omp atomic        (fast, single-op only)
//   counter_reduction — reduction(+:sum)          (fastest; per-thread privates)
//
// All three return the same number; the teaching point is the cost gap.
// The reduction version is the canonical idiom; critical and atomic shown
// for exposition.

// snippet-begin: critical
long long counter_critical(const std::vector<int>& v)
{
    long long sum = 0;
#pragma omp parallel for default(none) shared(v, sum)
    for (std::size_t i = 0; i < v.size(); ++i) {
#pragma omp critical
        {
            sum += v[i];
        }
    }
    return sum;
}
// snippet-end: critical

// snippet-begin: atomic
long long counter_atomic(const std::vector<int>& v)
{
    long long sum = 0;
#pragma omp parallel for default(none) shared(v, sum)
    for (std::size_t i = 0; i < v.size(); ++i) {
#pragma omp atomic
        sum += v[i];
    }
    return sum;
}
// snippet-end: atomic

// snippet-begin: reduction
long long counter_reduction(const std::vector<int>& v)
{
    long long sum = 0;
#pragma omp parallel for default(none) shared(v) reduction(+ : sum)
    for (std::size_t i = 0; i < v.size(); ++i) {
        sum += v[i];
    }
    return sum;
}
// snippet-end: reduction
