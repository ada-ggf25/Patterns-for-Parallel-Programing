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

The first parallel region pays a one-time team-creation cost (~µs). Without warm-up, the first measurement can be 5–10× slower than steady state:

```cpp
// Warm-up — pays startup cost once, result discarded
#pragma omp parallel { /* trivial work */ }

// Now measure steady state
const double t0 = omp_get_wtime();
#pragma omp parallel for ...
for (...) { ... }
const double t1 = omp_get_wtime();
```

The warm-up loop should also touch the same memory as the timed loop to prime NUMA placement and caches.

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
