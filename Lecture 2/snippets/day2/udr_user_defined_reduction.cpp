// OpenMP `#pragma omp declare reduction` lets you compose any
// associative+commutative operator over any type into a reduction
// clause — not just the built-in scalar arithmetic ones.
//
// Built-in `reduction(+:x)` works on scalars. To reduce a *compound*
// state (here: count + sum + sum-of-squares so we can compute mean
// and variance in one pass), the built-in operators are not enough.
// A user-defined reduction over a `Stats` struct does it in one
// parallel pass.

#include <cstddef>
#include <omp.h>
#include <vector>

// snippet-begin: educational
struct Stats {
    long n;
    double sum;
    double sum_sq;
};

#pragma omp declare reduction(stats_plus : Stats :                                                 \
                                  omp_out = {omp_out.n + omp_in.n,                                 \
                                             omp_out.sum + omp_in.sum,                             \
                                             omp_out.sum_sq + omp_in.sum_sq})                      \
    initializer(omp_priv = {0, 0.0, 0.0})

Stats parallel_stats(const std::vector<double>& v)
{
    Stats s{0, 0.0, 0.0};
    const std::size_t n = v.size();
#pragma omp parallel for default(none) shared(v, n) reduction(stats_plus : s)
    for (std::size_t i = 0; i < n; ++i) {
        s.n      += 1;
        s.sum    += v[i];
        s.sum_sq += v[i] * v[i];
    }
    return s;
}
// snippet-end: educational
