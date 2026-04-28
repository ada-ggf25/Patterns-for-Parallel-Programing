// `firstprivate` initialises the per-thread copy from the value at the
// region entry. Reach for it sparingly — most uses of "I want this to
// start at the outer value" are actually `shared` reads-only or a
// reduction, both of which are clearer.
//
// Two contrasted patterns below.

#include <cstddef>
#include <omp.h>
#include <vector>

// snippet-begin: pattern_right
// Pattern 1 — `firstprivate` is correct here: each thread needs its
// own running scratch initialised from a configured starting value.
void simulate_with_seed(std::vector<double>& out, double seed)
{
    const std::size_t n = out.size();
#pragma omp parallel for default(none) shared(out, n) firstprivate(seed)
    for (std::size_t i = 0; i < n; ++i) {
        double scratch = seed;        // each iter starts at `seed`
        scratch = (scratch * 1.01) + static_cast<double>(i);
        out[i] = scratch;
    }
}
// snippet-end: pattern_right

// snippet-begin: pattern_wrong
// Pattern 2 — `firstprivate` would be the WRONG reach here. The outer
// scalar is read-only; `shared` is fine and signals intent more
// clearly than a per-thread copy nobody mutates.
double weighted_sum(const std::vector<double>& v, double weight)
{
    double sum = 0.0;
    const std::size_t n = v.size();
#pragma omp parallel for default(none) shared(v, n, weight) reduction(+ : sum)
    for (std::size_t i = 0; i < n; ++i) {
        sum += v[i] * weight;         // weight is read-only; shared is enough
    }
    return sum;
}
// snippet-end: pattern_wrong
