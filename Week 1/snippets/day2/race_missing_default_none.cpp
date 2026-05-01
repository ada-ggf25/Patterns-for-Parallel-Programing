// SAFETY: intentional race for teaching purposes (ThreadSanitizer
// demo). Without `default(none)`, captured automatic variables default
// to `shared`. Many concurrent unsynchronised increments on a shared
// scalar = a race; TSan flags it.
//
// Fix: add `default(none) shared(...)` / `reduction(...)` and have the
// compiler force you to enumerate every captured variable.

#include <omp.h>

// snippet-begin: races
long count_races(int iterations)
{
    long counter = 0;  // shared by default — race target
#pragma omp parallel  // NOLINT — pedagogical: deliberately omits default(none)
    {
        for (int i = 0; i < iterations; ++i) {
            ++counter;  // unsynchronised RMW on shared `counter`
        }
    }
    return counter;
}
// snippet-end: races

// The fix: declare the intent, then use a reduction.
// snippet-begin: safely
long count_safely(int iterations)
{
    long counter = 0;
#pragma omp parallel default(none) shared(iterations) reduction(+ : counter)
    {
        for (int i = 0; i < iterations; ++i) {
            ++counter;
        }
    }
    return counter;
}
// snippet-end: safely
