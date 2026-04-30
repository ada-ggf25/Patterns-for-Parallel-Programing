# Timing with `omp_get_wtime`

The canonical OpenMP timing idiom. Returns wall-clock seconds since an implementation-defined origin; only differences are meaningful.

## Basic usage

```cpp
#include <omp.h>

const double t0 = omp_get_wtime();
#pragma omp parallel for ...
for (...) { ... }
const double t1 = omp_get_wtime();
const double duration_s = t1 - t0;
```

- Reads are **outside** the parallel region — wall-clock includes all threading overhead.
- **Do not** use `std::clock()` — it sums CPU time across threads, not wall time.
- **Do not** take per-thread timestamps inside the region.

## The warm-up trick

The first measurement of any kernel pays a stack of one-time costs the second one doesn't. Wrap the timed region with a warm-up that mirrors the real access pattern, then throw away its wall-clock:

```cpp
// Warm-up — mirrors the real access pattern; result discarded
#pragma omp parallel for ...
for (...) { /* same memory as timed loop */ }

// Now measure steady state
const double t0 = omp_get_wtime();
#pragma omp parallel for ...
for (...) { ... }
const double t1 = omp_get_wtime();
```

Why the warm-up matters — five distinct costs:

- **NUMA first-touch placement**: physical pages are allocated on the NUMA node of whichever thread first writes to them. Without a parallel warm-up, pages may sit on the main thread's node — remote-node access is 1.5–3× slower on Rome.
- **Page faults**: ~10 µs per 4 KB page; a 100 MB array costs ~0.5 s in fault overhead on the cold run.
- **CPU frequency / Turbo settling**: cores ramp clock from idle. Short kernels measured cold land below steady-state clock.
- **TLB / cache warming**: marginal for streaming kernels (each iteration evicts the last), but can matter for smaller working sets.
- **OpenMP team creation**: tens of µs to spin up the worker threads on the first region.

## Min-of-k for noisy hardware

A single timing run on a shared cluster can be noisy (background jobs, OS jitter). Take the minimum of k runs:

```cpp
auto bench = [](auto fn) {
    double best = 1e30;
    for (int trial = 0; trial < 3; ++trial) {
        const double t0 = omp_get_wtime();
        fn();
        const double t1 = omp_get_wtime();
        best = std::min(best, t1 - t0);
    }
    return best;
};

const double t_impl_a = bench(impl_a);
const double t_impl_b = bench(impl_b);
```

`hyperfine` does this automatically and gives confidence intervals — use it for final benchmark runs on CX3.

## Comparing two implementations

```cpp
struct TimingResult { double fast_s; double slow_s; };

TimingResult compare(auto fn_a, auto fn_b) {
    const double ta = bench(fn_a);
    const double tb = bench(fn_b);
    if (ta < tb) return {ta, tb};
    return {tb, ta};
}
```

Both routes through the same timing harness → directly comparable, same warm-up cost.

## `hyperfine` (preferred for final benchmarks)

```bash
hyperfine --warmup 1 --runs 5 \
  './pi_openmp' \
  --export-json perf-results-a1.json
```

`--warmup 1` pays runtime startup once; `--runs 5` gives min/median/max. The exported JSON is required in assessment deliverables.

## Related

- [[Performance Metrics]] — what to do with the timing numbers.
- [[STREAM and HPL]] — STREAM uses its own timing, not `omp_get_wtime`.
- [[../openmp/OMP Environment Variables]] — set thread count and pinning before benchmarking.
