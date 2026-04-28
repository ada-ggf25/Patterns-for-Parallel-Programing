#include <cstddef>
#include <omp.h>
#include <vector>

// taskloop over an array-apply kernel: apply f(x) to each element in place.
//
// Two flavours:
//   apply_taskloop — uses `#pragma omp taskloop grainsize(G)` to create
//                    ceil(n/G) tasks, each covering G iterations.
//   apply_for      — equivalent `parallel for` baseline for comparison.
//
// For uniform-cost `f`, `apply_for` generally wins (lower overhead).
// For irregular `f`, `apply_taskloop` wins (dynamic scheduling).

static inline double f(double x)
{
    return (x * x) + (2.0 * x) + 1.0;
}

// snippet-begin: taskloop_form
void apply_taskloop(std::vector<double>& v, int grainsize)
{
    const std::size_t n = v.size();
#pragma omp parallel default(none) shared(v, n) firstprivate(grainsize)
    {
#pragma omp single
        {
#pragma omp taskloop grainsize(grainsize)
            for (std::size_t i = 0; i < n; ++i) {
                v[i] = f(v[i]);
            }
        }
    }
}
// snippet-end: taskloop_form

// snippet-begin: for_form
void apply_for(std::vector<double>& v)
{
    const std::size_t n = v.size();
#pragma omp parallel for default(none) shared(v, n)
    for (std::size_t i = 0; i < n; ++i) {
        v[i] = f(v[i]);
    }
}
// snippet-end: for_form
