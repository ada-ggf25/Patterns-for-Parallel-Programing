// `single`, `master`, and `nowait` — three constructs that often
// appear together inside parallel regions, with subtly different
// semantics.
//
//   #pragma omp single   — exactly ONE arbitrary thread runs the
//                          block; *implicit barrier at exit* unless
//                          `nowait` is appended.
//   #pragma omp masked   — only the master thread (id 0) runs the
//                          block; NO implicit barrier.
//   `nowait` clause      — strips the implicit barrier from a
//                          worksharing construct.
//
// `master` (deprecated in 5.1) is replaced by `masked` (which can
// designate any thread, defaulting to 0). We use `masked` here.

#include <omp.h>

// Idiomatic `single`: do one-shot setup, every other thread waits.
int single_then_team(int seed)
{
    int shared_bytes = 0;
    int per_thread_count = 0;
#pragma omp parallel default(none) shared(shared_bytes, per_thread_count) firstprivate(seed)
    {
#pragma omp single
        {
            shared_bytes = seed * 4;          // setup, runs once
        }
        // implicit barrier: every thread sees `shared_bytes` here

#pragma omp atomic
        per_thread_count++;
    }
    return shared_bytes + per_thread_count;
}

// `single nowait`: setup runs once, but the team does NOT wait.
// Useful when the work that follows doesn't depend on the setup —
// e.g. reading a small constant from elsewhere. Be sure!
int single_nowait_independent(int seed)
{
    int unrelated = 0;
    int setup_done = 0;
#pragma omp parallel default(none) shared(unrelated, setup_done) firstprivate(seed)
    {
#pragma omp single nowait
        {
            setup_done = seed;          // not read by any thread below
        }
        // no barrier — fast threads start straight away

#pragma omp atomic
        unrelated++;
    }
    return unrelated + setup_done;
}

// `masked` — only thread 0, no barrier. Use sparingly: most of the
// time `single` is what you want because it carries the barrier.
int masked_demo()
{
    int once = 0;
    int many = 0;
#pragma omp parallel default(none) shared(once, many)
    {
#pragma omp masked
        {
            once = 42;       // only thread 0; no implicit barrier
        }
#pragma omp atomic
        many++;
    }
    return once + many;
}
