#include <array>
#include <cstddef>
#include <omp.h>

// Demonstrates false sharing — two thread-local accumulators are *legally*
// independent but land on the same cache line, so every write invalidates
// the other's copy.
//
// `bad_accum`  — 8-byte values packed adjacently (per-thread stride = 8 B)
//               → cache-line ping-pong at 64-byte granularity.
// `good_accum` — each value aligned to a cache line (64-byte stride)
//               → no sharing, scales cleanly.
//
// Both return the total; only the timing differs. The A3 slides include a
// live demo rendering the speedup gap at 64 threads on Rome.

// snippet-begin: layouts
struct alignas(64) Padded {
    double v;
    char pad[56];  // filler so sizeof(Padded) is a whole cache line
};
// snippet-end: layouts

static constexpr std::size_t NT_MAX = 128;

// snippet-begin: bad_accum
double bad_accum(int iters_per_thread)
{
    std::array<double, NT_MAX> buckets{};

#pragma omp parallel default(none) shared(buckets, iters_per_thread)
    {
        const int tid = omp_get_thread_num();
        for (int i = 0; i < iters_per_thread; ++i) {
            buckets[static_cast<std::size_t>(tid)] += 1.0;
        }
    }

    double total = 0.0;
    for (double b : buckets) {
        total += b;
    }
    return total;
}
// snippet-end: bad_accum

// snippet-begin: good_accum
double good_accum(int iters_per_thread)
{
    std::array<Padded, NT_MAX> buckets{};

#pragma omp parallel default(none) shared(buckets, iters_per_thread)
    {
        const int tid = omp_get_thread_num();
        for (int i = 0; i < iters_per_thread; ++i) {
            buckets[static_cast<std::size_t>(tid)].v += 1.0;
        }
    }

    double total = 0.0;
    for (const auto& b : buckets) {
        total += b.v;
    }
    return total;
}
// snippet-end: good_accum
