#include <cstddef>
#include <omp.h>

// A small routine that mimics the A1 workload: non-uniform per-iteration
// cost, so `schedule(static)`, `schedule(dynamic)`, and `schedule(guided)`
// produce measurably different timings.
//
// `cost(i)` returns how many inner iterations to burn at index `i`. For
// demonstration purposes we use a spike: most iterations cheap, a tenth
// expensive. Real A1 `f(x)` has a similar shape (chosen deliberately).
static inline int cost(std::size_t i)
{
    return (i % 10 == 0) ? 2000 : 50;
}

static inline double busy_work(int iters)
{
    double s = 0.0;
    for (int k = 0; k < iters; ++k) {
        s += 1.0 / (1.0 + (0.001 * static_cast<double>(k)));
    }
    return s;
}

// Three sibling functions share the same kernel body but differ in schedule.
// Callers time them to compare.

// snippet-begin: static
double sum_static(std::size_t n)
{
    double sum = 0.0;
#pragma omp parallel for default(none) shared(n) reduction(+ : sum) schedule(static)
    for (std::size_t i = 0; i < n; ++i) {
        sum += busy_work(cost(i));
    }
    return sum;
}
// snippet-end: static

// snippet-begin: dynamic
double sum_dynamic_64(std::size_t n)
{
    double sum = 0.0;
#pragma omp parallel for default(none) shared(n) reduction(+ : sum) schedule(dynamic, 64)
    for (std::size_t i = 0; i < n; ++i) {
        sum += busy_work(cost(i));
    }
    return sum;
}
// snippet-end: dynamic

// snippet-begin: guided
double sum_guided(std::size_t n)
{
    double sum = 0.0;
#pragma omp parallel for default(none) shared(n) reduction(+ : sum) schedule(guided)
    for (std::size_t i = 0; i < n; ++i) {
        sum += busy_work(cost(i));
    }
    return sum;
}
// snippet-end: guided
