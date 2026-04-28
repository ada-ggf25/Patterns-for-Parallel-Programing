#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <omp.h>

int single_then_team(int seed);
int single_nowait_independent(int seed);
int masked_demo();

TEST_CASE("single sets up shared state visible to every thread")
{
    constexpr int seed = 5;
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const int got = single_then_team(seed);
        // shared_bytes = seed*4 = 20; per_thread_count = p
        CHECK(got == 20 + p);
    }
}

TEST_CASE("single nowait still runs the body exactly once")
{
    constexpr int seed = 9;
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const int got = single_nowait_independent(seed);
        // setup_done = 9 (single), unrelated = p
        CHECK(got == 9 + p);
    }
}

TEST_CASE("masked block runs on thread 0 only")
{
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const int got = masked_demo();
        // once = 42 (thread 0 only); many = p
        CHECK(got == 42 + p);
    }
}
