#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cstddef>
#include <omp.h>

long bucketed_count(std::size_t n_buckets, int events_per_thread);

TEST_CASE("bucketed_count returns p * events_per_thread for any p")
{
    constexpr int events = 1000;
    for (int p : {1, 2, 4}) {
        omp_set_num_threads(p);
        const long got = bucketed_count(/*n_buckets=*/16, events);
        CHECK(got == static_cast<long>(p) * events);
    }
}
