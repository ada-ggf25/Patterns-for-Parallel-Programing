# False Sharing

False sharing is when threads write to *logically independent* data that happens to reside on the **same cache line**. The CPU's cache coherence protocol (MESI) treats the line as a unit and invalidates all copies on every write — even if different threads are writing different bytes.

## Cache line granularity

CPUs move data in 64-byte cache lines (on Rome and virtually all modern x86-64). Eight `double` values (8 B each) pack into one line:

```
address: 0x..40                                  0x..80
         ┌──────────────────────────────────────┐
         │ d0 │ d1 │ d2 │ d3 │ d4 │ d5 │ d6 │ d7 │  one 64 B cache line
         └──────────────────────────────────────┘
```

If thread 0 writes `d0` and thread 1 writes `d1`, both writes invalidate each other's copy of the entire line — even though no data was actually shared.

## The MESI protocol

| State | Meaning |
|---|---|
| **Modified** | Dirty, owned by this cache only |
| **Exclusive** | Clean, owned by this cache only |
| **Shared** | Read-only, possibly in multiple caches |
| **Invalid** | Stale — must re-fetch from memory |

A write to a **Shared** line transitions it to **Modified** in this cache and sends invalidation to all other copies. The other cores' next read misses → cache-line bounce at ~100 ns on Rome.

## Bad layout: packed accumulators

```cpp
// Each thread writes buckets[tid] — looks independent, but shares cache lines
std::array<double, 128> buckets{};
#pragma omp parallel default(none) shared(buckets, iters_per_thread)
{
    const int tid = omp_get_thread_num();
    for (int i = 0; i < iters_per_thread; ++i)
        buckets[tid] += 1.0;   // every write invalidates 7 neighbours' copies
}
```

8 threads' slots pack onto one 64-byte line → every write triggers MESI invalidation × 7. Performance collapses to ~1 write per memory round-trip.

## Good layout: cache-line padding

```cpp
struct alignas(64) Padded {
    double v;
    char pad[56];   // filler: sizeof(Padded) == 64 = one cache line
};

std::array<Padded, 128> buckets{};
#pragma omp parallel default(none) shared(buckets, iters_per_thread)
{
    const int tid = omp_get_thread_num();
    for (int i = 0; i < iters_per_thread; ++i)
        buckets[tid].v += 1.0;   // own cache line — no sharing
}
```

`alignas(64)` guarantees each `Padded` is on its own line. Zero inter-thread traffic. Typically **5–10× faster** at 64+ threads for this workload, with identical code in the loop body.

## Diagnosis

False sharing shows up as:
- Scaling that plateaus early (especially 4–8×) even when load is balanced.
- High L2/L3 miss rates in performance counters.
- Speedup that gets *worse* as thread count increases past a threshold.

The fix is always a data layout change — pad to cache-line boundaries.

## A3 extension

A3's false-sharing extension asks you to implement both a `before` (packed) and `after` (padded) variant and measure the delta. Soft threshold: `delta_percent ≥ 15` → full marks.

## Related

- [[NUMA First Touch]] — the other layout-driven performance issue in A3.
- [[../openmp/Data Races and TSan]] — false sharing is a performance bug, not a race, but the symptoms can look similar.
- [[../assessment/A3 Jacobi]] — A3-extension/false_sharing.
