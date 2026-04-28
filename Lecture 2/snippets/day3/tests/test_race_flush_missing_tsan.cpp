// TSan-lane test for race_flush_missing.cpp.
//
// Exercises the buggy publish_bad / consume_bad pair in a loop. Without
// release/acquire semantics the OpenMP memory model permits the
// reader to observe ready=1 with payload=0; TSan + Archer flag this
// as a missing flush and halt.
//
// Excluded from the release lane via the RACE_DEMO ctest LABEL.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <atomic>
#include <omp.h>

struct ChannelBad {
    std::atomic<int> ready;
    int payload;
};
void publish_bad(ChannelBad& c, int v);
int consume_bad(ChannelBad& c);

TEST_CASE("publish_bad / consume_bad provoke the visibility race for TSan")
{
    omp_set_num_threads(2);
    for (int trial = 0; trial < 50; ++trial) {
        ChannelBad c{{0}, 0};
        int seen = -1;
#pragma omp parallel sections default(none) shared(c, seen)
        {
#pragma omp section
            {
                publish_bad(c, 7);
            }
#pragma omp section
            {
                seen = consume_bad(c);
            }
        }
        // No numerical assertion — under TSan, halt_on_error=1 fires
        // before we get here; under release this test does not run.
        (void)seen;
    }
}
