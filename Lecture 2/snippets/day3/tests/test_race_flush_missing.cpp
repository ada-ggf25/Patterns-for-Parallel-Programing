#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <atomic>
#include <omp.h>

struct ChannelBad {
    std::atomic<int> ready;
    int payload;
};
struct ChannelGood {
    int ready;
    int payload;
};

void publish_bad(ChannelBad& c, int v);
int consume_bad(ChannelBad& c);
void publish_good(ChannelGood& c, int v);
int consume_good(ChannelGood& c);

// Release lane only checks the *correct* implementation. The buggy
// version's behaviour is undefined; the TSan/Archer lane (Phase 3)
// is what proves the bug.
TEST_CASE("publish_good / consume_good round-trip the value")
{
    omp_set_num_threads(2);
    for (int v : {1, 7, 42}) {
        ChannelGood c{};
        int seen = -1;
#pragma omp parallel sections default(none) shared(c, seen) firstprivate(v)
        {
#pragma omp section
            {
                publish_good(c, v);
            }
#pragma omp section
            {
                seen = consume_good(c);
            }
        }
        CHECK(seen == v);
    }
}
