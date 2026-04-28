#include <omp.h>

// A didactic Fibonacci using OpenMP tasks.
//
// This is deliberately NOT an efficient way to compute Fibonacci — the point
// is to show the task spawn idiom (`#pragma omp task` + `#pragma omp taskwait`)
// and the need for a `single` block to avoid every thread redundantly
// spawning the same work.
//
// Realistic use case: recursive divide-and-conquer problems (quicksort,
// Barnes–Hut tree walks, mergesort).
// snippet-begin: recursion
long long fib_task(long long n)
{
    if (n < 2) {
        return n;
    }

    long long x = 0;
    long long y = 0;

#pragma omp task shared(x) firstprivate(n)
    x = fib_task(n - 1);
#pragma omp task shared(y) firstprivate(n)
    y = fib_task(n - 2);
#pragma omp taskwait

    return x + y;
}
// snippet-end: recursion

// snippet-begin: driver
long long fib_parallel(long long n)
{
    long long result = 0;
#pragma omp parallel default(none) shared(result) firstprivate(n)
    {
#pragma omp single
        result = fib_task(n);
    }
    return result;
}
// snippet-end: driver
