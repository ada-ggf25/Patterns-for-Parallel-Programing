// Explicit and implicit barriers in OpenMP.
//
// Implicit barriers exist at the *end* of `for`, `single`, `sections`,
// and the parallel region itself. `nowait` removes the implicit barrier
// from a worksharing construct so threads continue to the next stage
// without waiting for the slowest in the team.
//
// `#pragma omp barrier` is the explicit form — every thread must reach
// it before any thread proceeds.

#include <cstddef>
#include <omp.h>
#include <vector>

// Two-stage compute: produce `a`, then consume it into `b`. We MUST
// keep the implicit barrier because the second loop reads what the
// first wrote, and `parallel for` may distribute iterations differently
// across the two `for` constructs (the spec does not guarantee a
// matching schedule by default).
void produce_then_consume(std::vector<double>& a, std::vector<double>& b)
{
    const std::size_t n = a.size();
#pragma omp parallel default(none) shared(a, b, n)
    {
#pragma omp for
        for (std::size_t i = 0; i < n; ++i) {
            a[i] = static_cast<double>(i);
        }
        // Implicit barrier — required because of the data dependence.

#pragma omp for
        for (std::size_t i = 0; i < n; ++i) {
            b[i] = a[i] * 2.0;
        }
    }
}

// Two *independent* computations: `a` is filled from a constant; `b`
// is filled from a different constant; nothing the first loop writes
// is read by the second. `nowait` is safe — fast threads start the
// second loop without waiting.
void independent_fills(std::vector<double>& a, std::vector<double>& b)
{
    const std::size_t n_a = a.size();
    const std::size_t n_b = b.size();
#pragma omp parallel default(none) shared(a, b, n_a, n_b)
    {
#pragma omp for nowait
        for (std::size_t i = 0; i < n_a; ++i) {
            a[i] = 1.0;
        }
        // No barrier — the next loop has no dependence on `a`.

#pragma omp for
        for (std::size_t j = 0; j < n_b; ++j) {
            b[j] = 2.0;
        }
    }
}

// Explicit barrier — useful inside hand-rolled phases where there is
// no convenient `for` to attach a barrier to.
int explicit_barrier_demo()
{
    int phase_a_done = 0;
    int phase_b_done = 0;
#pragma omp parallel default(none) shared(phase_a_done, phase_b_done)
    {
#pragma omp atomic
        phase_a_done++;

#pragma omp barrier  // every thread must reach here before continuing

#pragma omp atomic
        phase_b_done++;
    }
    return phase_a_done + phase_b_done;
}
