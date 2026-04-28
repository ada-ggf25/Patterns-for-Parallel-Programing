// SAFETY: intentional race for teaching purposes.
//
// Demonstrates an OpenMP-5.1 memory-model race: one thread sets a
// "ready" flag and writes data; another spins on the flag and reads
// the data. Without `flush` (or 5.1 acquire/release atomics), the
// reader can observe `ready=1` *before* the data write becomes
// visible — classic publish/subscribe ordering bug.
//
// This is NOT a data-race in the C++ sense for atomic_int, but it IS
// an OpenMP visibility race for the non-atomic `payload`. TSan
// (with Archer/OMPT) flags the missing flush; the matching test runs
// in the TSan lane only.
//
// The fix at bottom uses 5.1 `atomic write release` / `atomic read
// acquire` so that the payload write happens-before the flag store.

#include <atomic>
#include <omp.h>

// === Buggy version: no flush, no acquire/release. ===
// snippet-begin: bad
struct ChannelBad {
    std::atomic<int> ready{0};
    int payload{0};            // *not* atomic; relies on visibility
};

void publish_bad(ChannelBad& c, int v)
{
    c.payload = v;             // (1) non-atomic write
    c.ready.store(1);          // (2) seq_cst store of the flag
    // OpenMP runtimes may or may not flush (1) before (2) is observed.
}

int consume_bad(ChannelBad& c)
{
    while (c.ready.load() == 0) {
        // spin
    }
    return c.payload;          // may observe 0 if (1) hasn't propagated
}
// snippet-end: bad

// === Fixed version: 5.1 release/acquire on the flag. ===
// snippet-begin: good_publisher
struct ChannelGood {
    int ready{0};
    int payload{0};
};

void publish_good(ChannelGood& c, int v)
{
    c.payload = v;
#pragma omp atomic write release
    c.ready = 1;               // release: payload write is visible to a
                               //          matching acquire-loader
}
// snippet-end: good_publisher

// snippet-begin: good_subscriber
int consume_good(ChannelGood& c)
{
    int seen = 0;
    do {
#pragma omp atomic read acquire
        seen = c.ready;        // acquire: subsequent reads see what
                               //          the matching release published
    } while (seen == 0);
    return c.payload;
}
// snippet-end: good_subscriber
