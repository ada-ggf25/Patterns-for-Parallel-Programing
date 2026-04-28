# A1 — Numerical Integration (20 pts)

Parallelise a composite-trapezoid integration kernel with **non-uniform per-iteration cost**, then analyse schedule performance across the canonical thread ladder.

## Kernel

Integrate `f(x)` over `[0, 1]` with N subintervals using the midpoint rule. `f(x)` has a deliberate **spike region** for `x ∈ [0.3, 0.4]` running ~10× slower than elsewhere — schedule choice is genuinely informative.

```cpp
double sum = 0.0;
#pragma omp parallel for default(none) shared(n, w) reduction(+:sum) schedule(???)
for (long long i = 1; i <= n; ++i) {
    const double x = w * (static_cast<double>(i) - 0.5);
    sum += f(x);   // f(x) is cheap outside [0.3, 0.4], expensive inside
}
```

### Why the spike creates load imbalance

With `schedule(static)`, each thread gets a contiguous chunk of iterations. The spike iterations (i ≈ 0.3N to 0.4N) all land on one or two threads. Those threads take ~10× longer than the others; the fast threads finish and idle at the implicit barrier:

```
Thread 0: ██ done ——————————————— wait
Thread 1: █████████████████████████ (has the spikes)
Thread 2: ██ done ——————————————— wait
...
Thread 127: ██ done —————————————— wait
```

`dynamic` or `guided` distribute work on demand so expensive iterations spread across the team.

## Skills Exercised

- `parallel for` + `reduction(+:sum)` + `default(none)`
- Schedule selection: `static`, `dynamic, C`, `guided` — chosen by measurement
- Chunk-size tuning for `schedule(dynamic, C)`
- Timing with `omp_get_wtime` + warm-up + min-of-k
- Computing speedup and efficiency from measured times
- Roofline reasoning for a compute-bound kernel

## Deliverables

| Path | What |
|---|---|
| `assignment-1/integrate.cpp` | Parallel implementation |
| `assignment-1/answers.csv` | 15 MCQ answers (one `A`/`B`/`C`/`D` per row) |
| `assignment-1/tables.csv` | Times + speedups + efficiencies at `{1, 16, 64, 128}` |
| `assignment-1/REFLECTION.md` | 4 sections + reasoning question (≥ 50 words each) |
| `assignment-1/perf-results-a1.json` | `hyperfine` output from CX3 benchmark |

### `tables.csv` exact format

```
thread_count,measured_time_s,measured_speedup,measured_efficiency
1,<T1>,1.00,1.00
16,<T16>,<T1/T16>,<(T1/T16)/16>
64,<T64>,<T1/T64>,<(T1/T64)/64>
128,<T128>,<T1/T128>,<(T1/T128)/128>
```

- `measured_speedup = T(1) / T(P)` — must be within 2 % of that formula.
- `measured_efficiency = speedup / P` — same tolerance.
- The grader does an **internal consistency check only**: it verifies your own numbers are arithmetically coherent, not that they match instructor times.
- Fill in all three columns for each row. Leaving blanks → internal-consistency check fails.

### Generating `perf-results-a1.json` on CX3

```bash
# From the assessment repo root on CX3, submit the provided PBS script:
qsub assignment-1/evaluate.pbs

# Or run hyperfine manually inside a job:
export OMP_NUM_THREADS=128
export OMP_PLACES=cores
export OMP_PROC_BIND=close
hyperfine --warmup 1 --runs 5 \
  './integrate' \
  --export-json assignment-1/perf-results-a1.json
```

The PBS script already sets the right thread counts and exports the combined JSON. See [[../examples/Running on CX3]] and the `docs/cx3-benchmarking.md` guide for full submission steps.

## Scoring (20 pts)

| Component | Pts | Check |
|---|---|---|
| Build + TSan clean | 2 | Clang-18 + TSan + Archer |
| Correctness at `{1, 16, 64, 128}` | 6 | 1.5 pts per thread count via `smart_diff.py` |
| Reference-parallel-time at 128T | 5 | `min(1.0, T_ref(128) / T_student(128)) × 5` |
| `tables.csv` internal consistency | 1 | `speedup = T(1)/T(P)`, `eff = speedup/P` within 2 % |
| Style (clang-format / clang-tidy / cppcheck) | 2 | Lint workflow |
| MCQ (15 questions) | 2 | Key comparison on `answers.csv` |
| REFLECTION format + completion | 1 | Header presence + ≥ 50 words per section |
| Reasoning question (instructor-marked) | 1 | Manual 0 / 0.5 / 1 |

### Reference-parallel-time scoring

$$\text{score} = \min\!\left(1.0,\; \frac{T_{\text{ref}}(128)}{T_{\text{student}}(128)}\right) \times 5$$

`T_ref` is measured by the instructor on Rome with the reference solution and **published once at the start of the cohort** — check `rubric.md` for the current value. The scoring is **correctness-gated**: any thread-count correctness failure → 0 perf score.

### Correctness check

`smart_diff.py` compares your integration result against the reference at each of `{1, 16, 64, 128}` threads. It uses a relative tolerance of ~1e-9 to allow normal floating-point rounding from different reduction orders. A race condition produces results that vary between runs and will fail the check.

## Schedule Sweep Methodology

Measure all three at each thread count. Don't assume the same winner everywhere.

```cpp
const double t_static  = time(sum_static, n);        // schedule(static)
const double t_dynamic = time(sum_dynamic_64, n);    // schedule(dynamic, 64)
const double t_guided  = time(sum_guided, n);         // schedule(guided)
```

### Chunk size tuning (`dynamic, C`)

| C value | Problem |
|---|---|
| C = 1 | Queue-dispatch overhead per chunk dominates at high thread counts |
| C = N/P | Same as static — spikes cluster on one thread |
| C = 64 | Good starting point for spike-every-10% workload |

Rule of thumb: chunk should be large enough that dispatch overhead is < 5 % of chunk compute time, but small enough that each thread handles several chunks (so load balances). For spike workloads, **C = 32–128 is typical**. Measure and record.

### Schedule winners by thread count

| Thread count | Typical winner | Why |
|---|---|---|
| 1 | All tie | Single thread; no scheduling at all |
| 16 | `dynamic` or `guided` | Spike is 10 % of iterations; static gives one thread all of them |
| 64 | `dynamic, 64` | More threads → imbalance worse; dynamic distributes best |
| 128 | `dynamic, 64` | Same, amplified; static efficiency drops to ~10-15 % |

At 128 threads, `static` efficiency collapse is most visible because each thread has a small chunk — if that chunk contains any spikes, the load imbalance is maximally apparent.

## REFLECTION Guide

Each section requires ≥ 50 words and must be grounded in your measured numbers.

### Section 1 — Schedule choice and why

Answer: which schedule + chunk size you chose, and why the measurements supported it.

Structure:
1. Name the winner (e.g., `schedule(dynamic, 64)`).
2. Explain the cost structure of f(x) — spike region means non-uniform per-iteration cost.
3. State what static gave you: times at each thread count, then note the schedule you tried and discarded.
4. Explain why dynamic/guided wins: load rebalancing distributes the spike across all threads.

> Example framing: "Static gave 2.4 s at 128 threads while dynamic (chunk=64) gave 0.6 s — a 4× gap. The spike region clusters in the range i ≈ 0.3N–0.4N; with static these fall on threads 38–51, creating a ~10× imbalance. Dynamic eliminates this because threads pull the next chunk immediately on completion."

### Section 2 — Scaling behaviour

Answer: where speedup departs from ideal (linear), and why.

Structure:
1. Identify the breakpoint: e.g., "speedup is near-linear up to 16T, then flattens."
2. Attribute to a cause: Amdahl's serial fraction, load imbalance (if static), NUMA effects at 64T+.
3. Report efficiency values from your `tables.csv` — e.g., "efficiency drops from 0.92 at 16T to 0.61 at 128T."
4. For a well-parallelised integration kernel with dynamic schedule, expect:
   - 16T: ~85–95 % efficiency (very good)
   - 64T: ~70–85 % (NUMA starts to matter)
   - 128T: ~55–75 % (cross-socket NUMA penalty)

The departure from linear speedup comes from:
- **Fork-join overhead** (constant cost, diminishing return as P grows)
- **NUMA** — at 64T the loop spans both sockets; remote-DRAM reads cost 32/12 ≈ 2.7× more
- **Memory bandwidth** — at 128T the integration reads N doubles of input; bandwidth gets shared
- **Reduction cost** — combining 128 partial sums (log₂128 = 7 steps)

### Section 3 — Roofline position

A1 integration is **compute-bound** (OI >> ridge point). The section asks you to estimate where on the roofline your kernel sits.

**Rome roofline constants (canonical):**
- Peak DP (theoretical): 4608 GFLOPs (128 cores × 2.25 GHz × 16 FLOPs/cycle AVX2 FMA)
- HPL-achievable compute ceiling: 2896 GFLOPs
- STREAM Triad (128T full-node): 231.5 GB/s
- Ridge OI: 4608 / 246 ≈ **18.7 FLOPs/byte**

**How to estimate OI for f(x):**
1. Count FLOPs per call to f(x). The base case is a simple polynomial or transcendental — estimate 2–10 FLOPs. The spike region runs ~10× more inner iterations.
2. Estimate bytes accessed: f(x) reads `x` (8 bytes) from a register, no array access → memory traffic is negligible per iteration.
3. Result: OI >> 18.7 → **firmly compute-bound**.

**How to compute achieved GFLOPs:**
```
FLOPs_per_iter  = (your estimate of f(x) FLOPs, e.g. ~5 base / ~50 spike)
weighted_avg    = 0.9 * base_flops + 0.1 * spike_flops   # spike is 10% of range
total_FLOPs     = weighted_avg * N
achieved_GFLOPs = total_FLOPs / T(P) / 1e9
fraction        = achieved_GFLOPs / 4608   (or / 2896 for HPL ceiling)
```

**Expected result:** the fraction will be very low (likely < 5 %) because:
- The kernel is scalar (no SIMD/AVX2 vectorisation of f(x))
- The spike's inner loop defeats auto-vectorisation
- You're hitting the compute ceiling, but that ceiling is 4608 GFLOPs — a single floating-point add is ~16 FLOPs/cycle with AVX2 FMA, and a scalar add is 1 FLOPs/cycle

**What to write:** "A1 is compute-bound (OI ≈ X >> 18.7 ridge). At 128T I achieved ~Y GFLOPs = Z % of the 4608 GFLOPs theoretical peak. The low fraction reflects scalar execution of f(x); the kernel is compute-ceiling–limited but doesn't exploit SIMD."

### Section 4 — What you'd try next

Pick one concrete change. Good options:
- **SIMD-vectorise f(x)** using `#pragma omp simd` or explicit intrinsics. The spike's branch (`if x ∈ [0.3, 0.4]`) must be converted to a blending mask. Predicted effect: 4–8× throughput on the base case; limited to <2× on the spike because the branch kills SIMD width.
- **Tune chunk size** — profile dynamic(32) vs dynamic(128) at 128T to find the crossover point where dispatch overhead equals load-balance benefit.
- **Increase N** to ensure the kernel is not memory-latency limited (if N is small, the loop fits in L3 and you're measuring cache effects, not parallel scaling).

## MCQ (15 questions, answers.csv)

The MCQ tests conceptual understanding of A1 topics. Study these vault notes:

| Topic | Note |
|---|---|
| `parallel for` + reduction | [[../openmp/reduction clause]], [[../openmp/for directive]] |
| `default(none)` and scoping | [[../openmp/Variable Scoping]] |
| Schedule kinds and trade-offs | [[../openmp/Schedules]] |
| Speedup and efficiency formulas | [[../performance/Performance Metrics]] |
| Roofline and compute vs bandwidth | [[../performance/Roofline Model]] |
| Why scaling departs from linear | [[../openmp/Amdahls Law]], [[../performance/Six Sources of Overhead]] |
| omp_get_wtime warm-up | [[../performance/Timing omp_get_wtime]] |

Format of `answers.csv` — one answer per line, 15 lines total:
```
A
C
B
...
```

## Grading Branch and Commit Workflow

The grader snapshots your **`feature/eval`** branch at the end of day 5. Work on that branch from day 2.

Commit history is reviewed for academic integrity (not graded with marks). The grader flags:
- Fewer than 8 commits across fewer than 3 calendar days
- A single commit containing > 90 % of the total diff
- > 80 % of commits with low-quality messages (`fix`, `update`, `wip`, `< 15 chars`)
- 100 % of diff committed on the last day

Aim for one or two meaningful commits per working session with descriptive subjects. See `assessment/handouts/commit-history-guidance.md` for the full rules and examples.

## Common Mistakes

| Mistake | Effect | Fix |
|---|---|---|
| Forgetting `default(none)` | Implicit sharing of accumulator → race | Add `default(none) shared(n, w) reduction(+:sum)` |
| Not warming up before timing | First region pays runtime startup → T(1) looks 5–10× slower | Run a no-op parallel region before measurement |
| Same schedule for all thread counts without measuring | Misses that static collapses at 128T | Sweep all 3 + chunk sizes; record all |
| `efficiency = speedup` in tables.csv | Fails internal-consistency check | Divide speedup by P |
| Timing inside the parallel region | Measures thread skew, not wall time | Both `omp_get_wtime()` calls outside the region |
| Not running on a full Rome node | Shared login node times are noisy and wrong | Use `qsub` with `select=1:ncpus=128:cpu_type=rome:place=excl` |

## Related

- [[../openmp/Schedules]] — static / dynamic / guided in depth, chunk-size tuning.
- [[../openmp/reduction clause]] — the canonical fix for accumulators.
- [[../openmp/Variable Scoping]] — `default(none)` discipline.
- [[../performance/Timing omp_get_wtime]] — warm-up, min-of-k, hyperfine.
- [[../performance/Performance Metrics]] — speedup, efficiency, reference-parallel-time.
- [[../performance/Roofline Model]] — OI, compute vs bandwidth ceiling, Rome constants.
- [[../performance/Six Sources of Overhead]] — why scaling departs from linear.
- [[../openmp/Amdahls Law]] — serial-fraction ceiling on speedup.
- [[Assessment Overview]] — 100-pt rubric and grading formulas.
