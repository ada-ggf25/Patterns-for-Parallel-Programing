# A2 Benchmark Results — CX3 Rome

> **Complete — Phase 3.5 sweep + Phase 4 formal benchmark both done.**  
> Platform: CX3 Rome (AMD EPYC 7742, 128 cores, 2 sockets, SMT off).  
> Build: GCC 13.3.0, `-O3 -march=znver2 -mavx2 -fopenmp`. Harness: `hyperfine --warmup 1 --min-runs 3`.  
> All times are the `min` value from hyperfine JSON (most stable run). Grid: 5000 × 2500 (upper half, ×2).

---

## Phase 3.5 Sweep — Schedule Selection (complete)

**Job:** `a2_sweep.o2597440` | **Results:** `bench/sweep/`

### Schedule comparison for `parallel_for` at all thread counts

| Schedule | T(1) s | T(16) s | T(64) s | T(128) s | Notes |
|---|---|---|---|---|---|
| `static` | 6.703 | 1.201 | 0.365 | 0.196 | Poor balance — boundary rows create hot-spots |
| `dynamic,1` | 6.703 | 0.432 | 0.119 | **0.068** | ✅ **Winner** — 5000 chunks, saturates 128T |
| `dynamic,100` | 6.702 | 0.564 | 0.466 | 0.466 | Only 50 chunks → thread starvation at 128T |
| `guided` | 6.701 | 0.548 | 0.152 | 0.086 | Good but coarser initial chunks leave tail |

### `tasks grainsize=1` reference at all thread counts

| T(1) s | T(16) s | T(64) s | T(128) s | Notes |
|---|---|---|---|---|
| 6.559 | 0.576 | 0.455 | 0.458 | Only 50 tasks (outer loop) → starvation at ≥ 64T |

### Winner decision

**`schedule(dynamic, 1)` is the winning schedule for `parallel_for`.**

Hardcoded in `mandelbrot_for.cpp`:
```cpp
#pragma omp parallel for reduction(+ : outside) default(none) shared(J_HALF) \
    schedule(dynamic, 1)
```

At 128T: 0.068 s vs `guided` 0.086 s (+21 %), `static` 0.196 s (+188 %), `tasks` 0.458 s (+573 %).

### Scaling analysis — `parallel_for schedule(dynamic,1)` (from sweep)

T(1) reference = 6.703 s (min from `sweep-for-dynamic_1-1.json`)

| Threads | T(P) s | Speedup | Efficiency |
|---|---|---|---|
| 1 | 6.703 | 1.00 | 1.00 |
| 16 | 0.432 | 15.51 | 0.97 |
| 64 | 0.119 | 56.33 | 0.88 |
| 128 | 0.068 | 98.57 | 0.77 |

Near-linear scaling — efficiency holds above 0.77 even at 128T.

### Scaling analysis — `tasks grainsize=1 outer` (from sweep)

T(1) reference = 6.559 s (min from `sweep-tasks-gs1-1.json`)

| Threads | T(P) s | Speedup | Efficiency |
|---|---|---|---|
| 1 | 6.559 | 1.00 | 1.00 |
| 16 | 0.576 | 11.39 | 0.71 |
| 64 | 0.455 | 14.42 | 0.23 |
| 128 | 0.458 | 14.32 | 0.11 |

Tasks plateau beyond 16T. Root cause: `taskloop grainsize(1)` on the outer loop (50 iterations)
= 50 tasks total. With 128 threads, 78 threads receive no work → efficiency collapses.

---

## Phase 4 Formal Benchmark — Complete ✅

**Job:** `a2_mandelbrot.o2598408` | **Results:** `bench/evaluate/`  
**Note:** GCC 14.3.0 was loaded (evaluate.pbs requested 13.3.0; 14.3.0 was the available version on the node). Optimisation flags `-O3 -march=znver2 -mavx2` applied in both cases.

### `parallel_for` variant (formal evaluate.pbs)

| thread_count | measured_time_s | measured_speedup | measured_efficiency |
|---|---|---|---|
| 1 | 6.5982 | 1.00 | 1.00 |
| 16 | 0.4221 | 15.63 | 0.98 |
| 64 | 0.1144 | 57.68 | 0.90 |
| 128 | 0.0659 | 100.12 | 0.78 |

### `tasks` variant (formal evaluate.pbs)

| thread_count | measured_time_s | measured_speedup | measured_efficiency |
|---|---|---|---|
| 1 | 6.4334 | 1.00 | 1.00 |
| 16 | 0.5627 | 11.43 | 0.71 |
| 64 | 0.4409 | 14.59 | 0.23 |
| 128 | 0.4430 | 14.52 | 0.113 |

### Winner at 128T (formal)

| Variant | T(128) s | Speedup vs serial | Notes |
|---|---|---|---|
| `parallel_for` | **0.0659** | 100.1 × | Near-linear scaling; efficiency 0.78 |
| `tasks` | 0.4430 | 14.5 × | Plateaus at 16T; efficiency 0.11 |
| **Winner** | **`parallel_for`** | **6.72 × faster** | Consistent with sweep prediction |

### Consistency check against sweep predictions

| Metric | Sweep (3.5) | Formal (4) | Δ |
|---|---|---|---|
| `parallel_for` T(128) | 0.068 s | 0.0659 s | −3 % |
| `tasks` T(128) | 0.458 s | 0.4430 s | −3 % |

Results agree within 3 % — expected given different job IDs and node allocation times.

---

## Extracting times from hyperfine JSON (CX3)

After `evaluate.pbs` completes, the following files appear in the repo root:

```
mandelbrot-parallel_for-1.json    ← intermediate
mandelbrot-parallel_for-16.json   ← intermediate
mandelbrot-parallel_for-64.json   ← intermediate
mandelbrot-parallel_for-128.json  ← intermediate
mandelbrot-tasks-1.json           ← intermediate
mandelbrot-tasks-16.json          ← intermediate
mandelbrot-tasks-64.json          ← intermediate
mandelbrot-tasks-128.json         ← intermediate
perf-results-a2.json              ← MUST commit; grader cross-checks CHOICE.md against it
```

Only `perf-results-a2.json` is read by the grader.

```bash
python3 bin/hyperfine_min_time.py mandelbrot-parallel_for-1.json
python3 bin/hyperfine_min_time.py mandelbrot-parallel_for-128.json
python3 bin/hyperfine_min_time.py mandelbrot-tasks-1.json
python3 bin/hyperfine_min_time.py mandelbrot-tasks-128.json
```

---

## Schedule / grainsize notes

### `parallel_for` schedule selection (Phase 3.5 sweep — confirmed)

- `dynamic,1`: 5000 single-row chunks. At 128T: 5000 ≫ 128 → no thread starvation.
  Per-row cost variance is absorbed because individual rows can be reassigned as needed.
- `dynamic,100` (= `dynamic,TILE`): 50 chunks for 128 threads → 78 threads idle. Flat curve.
- `guided`: starts with large chunks, shrinks to 1. Good but coarser early assignments
  cause slight tail latency at extreme thread counts.
- `static`: assumes uniform cost; fails badly near the Mandelbrot boundary.

### `tasks` grainsize (Phase 3.5 — current implementation)

- `grainsize(1)` on outer loop (50 iterations) = 50 tasks, each processing 25 j-tiles.
- Saturates up to ~50 threads. 128T → 0.458 s = effectively limited by task count.
- To improve tasks at 128T, would need a flattened 1D loop over 1250 tiles with
  `grainsize(10)` → 125 tasks — closer to 1 task/thread but still sub-optimal.

---

## Related

- [[A2 Progress]] — current phase checklist.
- [[A2 Mandelbrot]] — full technical reference for both variants.
- [[../performance/Performance Metrics]] — speedup/efficiency formulas.
- [[../performance/Six Sources of Overhead]] — why efficiency drops at high thread counts.
- [[../openmp/Schedules]] — schedule selection guide.
- [[../openmp/taskloop]] — grainsize tuning guide.
