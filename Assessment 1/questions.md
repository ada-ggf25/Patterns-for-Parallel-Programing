# A1 — Multiple-choice questions

Fill in your answers in `answers.csv` (one letter per question, A/B/C/D).

## q01

Which clause makes the compiler error out if a variable's data-sharing attribute is not explicitly specified?

- A. `shared(none)`
- B. `default(none)`
- C. `private(all)`
- D. `explicit`

## q02

A `reduction(+:sum)` clause on a `parallel for`:

- A. Serialises updates to `sum`, creating a critical section.
- B. Gives each thread a private copy of `sum`, combining them at the end of the region.
- C. Requires `sum` to be declared `atomic`.
- D. Is only valid when the loop body contains a single statement.

## q03

A loop over `N=1_000_000` subintervals computes `f(x)` where `f` costs the same per-call for every `x`. Which schedule is most appropriate for static load balance and lowest overhead?

- A. `schedule(static)`
- B. `schedule(dynamic, 1)`
- C. `schedule(guided)`
- D. `schedule(runtime)`

## q04

`f(x)` cost varies by a factor of ~100 across the integration domain (some subintervals cheap, some expensive, in no predictable pattern). Best schedule?

- A. `schedule(static)`
- B. `schedule(static, 1)`
- C. `schedule(dynamic, 64)` or `schedule(guided)`
- D. `schedule(auto)`

## q05

Inside a parallel region, `omp_get_num_threads()` returns:

- A. The number of physical cores on the machine.
- B. The number of threads in the current team.
- C. `OMP_NUM_THREADS` regardless of context.
- D. 1 if called outside a parallel region (always).

## q06

Your A1 build is clean and `OMP_NUM_THREADS=128 ./integrate` produces the right result, but it runs in essentially the same wall-clock time as `OMP_NUM_THREADS=1 ./integrate`. Most likely cause?

- A. The kernel is bandwidth-bound and saturated already at 1 thread.
- B. A typo like `#pragma opm parallel for` — the compiler treats unknown pragmas as no-ops, so the code runs serial regardless of `OMP_NUM_THREADS`.
- C. The compiler optimised the loop away because the result is unused.
- D. `OMP_NUM_THREADS` is being clobbered by `OMP_PROC_BIND=close`.

## q07

Operational intensity for A1 is very high (small memory traffic per FLOP). Which roofline regime does A1 sit in?

- A. Compute-bound — ceiling is peak FLOPs.
- B. Bandwidth-bound — ceiling is STREAM bandwidth × OI.
- C. Latency-bound — ceiling is 1 / memory latency.
- D. I/O-bound — ceiling is disk bandwidth.

## q08

Day 2 cited two compute ceilings on Rome: theoretical peak **4608 GFLOPs** (from AVX2 FMA × 128 cores × 2.25 GHz) and HPL-achievable **2896 GFLOPs** (vendor-tuned DGEMM, 63 % of peak). Your A1 implementation reaches 200 GFLOPs at 128 threads. Which roofline fraction is the more honest framing of "how close are you to what real code can deliver"?

- A. 200 / 4608 ≈ 0.043 (ratio against theoretical peak — only meaningful if your code is DGEMM-class).
- B. 200 / 2896 ≈ 0.069 (ratio against HPL-achievable peak — the ceiling actually attained by carefully-tuned compute-bound code).
- C. Either is fine — they tell you the same thing.
- D. Neither — A1 is bandwidth-bound, so use STREAM as the ceiling.

## q09

Two students submit A1. Student X: serial time 4 s, 8-thread time 0.6 s. Student Y: serial time 2 s, 8-thread time 0.4 s. Whose implementation is better and why?

- A. X — higher speedup (6.67× vs 5×).
- B. Y — lower absolute time-to-solution at 8 threads.
- C. X — closer to perfect linear speedup.
- D. They are equivalent.

## q10

`firstprivate(x)`:

- A. Makes `x` shared across the team.
- B. Gives each thread a private copy of `x`, initialised from the value of `x` at region entry.
- C. Is deprecated in OpenMP 5.1.
- D. Only applies to integer-typed variables.

## q11

You add `reduction(+:sum)` and `default(none) shared(a, b) private(tid)` to a `parallel for`. Inside, you also write `c[i] = a[i] * b[i]`, where `c` is an array you forgot to list. What happens?

- A. `c` defaults to `shared`.
- B. `c` defaults to `private`.
- C. The compiler emits an error and won't build.
- D. The loop silently races on `c`.

## q12

You want one parallel pass over `std::vector<double>` that computes count, sum, and sum-of-squares — and you want the three accumulators to live in a single `Stats` struct so the rest of your code reads `.n`, `.sum`, `.sum_sq`. Which OpenMP construct lets you reduce the whole struct at once?

- A. `reduction(+:stats)` — the built-in `+` reduction works on any struct that has an overloaded `operator+`.
- B. `#pragma omp declare reduction(stats_plus : Stats : ...)` — declare a user-defined reduction over the struct, then list `reduction(stats_plus : s)` on the parallel for.
- C. There's no way to reduce a struct in OpenMP; use three separate scalar reductions.
- D. `#pragma omp critical` is the only correct option for compound state.

## q13

Your A1 self-speedup at 64 threads is 18× and at 128 threads is 19× — barely changed. Most likely cause?

- A. Compiler failed to vectorise the inner loop.
- B. Memory bandwidth saturation: A1 is bandwidth-bound at high thread counts.
- C. Schedule overhead: `dynamic, 1` chunk size is too small.
- D. Load imbalance from the spike region in `f(x)`: a few threads do most of the work, and adding more threads doesn't help if the spike isn't distributed.

## q14

`#pragma omp parallel for` is equivalent to which combined construct?

- A. `#pragma omp parallel` then `#pragma omp master` then a loop.
- B. `#pragma omp parallel` then `#pragma omp for` on the immediately-following loop.
- C. `#pragma omp sections` with one section per thread.
- D. `#pragma omp task` per loop iteration.

## q15

The `_OPENMP` macro is defined by the compiler when OpenMP is enabled. Its value for OpenMP 5.1 is:

- A. `51`
- B. `5.1`
- C. `202011`
- D. `2021`
