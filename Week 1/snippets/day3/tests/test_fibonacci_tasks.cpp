#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

long long fib_parallel(long long n);

// Reference closed-form serial fibonacci for the test.
static long long fib_serial(long long n)
{
    if (n < 2) {
        return n;
    }
    long long a = 0;
    long long b = 1;
    for (long long i = 2; i <= n; ++i) {
        const long long c = a + b;
        a = b;
        b = c;
    }
    return b;
}

TEST_CASE("task-parallel fibonacci matches serial reference")
{
    // Small n only — tasking recursion is O(2^n).
    for (long long n : {5LL, 10LL, 15LL, 20LL}) {
        omp_set_num_threads(4);
        CHECK(fib_parallel(n) == fib_serial(n));
    }
}
