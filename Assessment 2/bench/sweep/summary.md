# Sweep Results — Schedule / Grainsize Exploration

**Job:** `a2_sweep.o2597440` (PBS job ID 2597440)  
**Hardware:** CX3 Rome — AMD EPYC 7742, 128 physical cores, dual-socket, SMT off  
**Build:** GCC 13.3.0, `-O3 -march=znver2 -mavx2 -fopenmp`  
**Harness:** `hyperfine --warmup 1 --min-runs 3` (more runs at higher thread counts)  
**Grid:** 5000 × 2500 upper half, ×2 symmetry  
**Log:** `logs/a2_sweep.o2597440`

---

## 1. `parallel_for` — Schedule Sweep

All times are the **minimum** wall-clock value from the hyperfine JSON (most stable run).

| Schedule | T(1) s | T(16) s | T(64) s | T(128) s |
|---|---|---|---|---|
| `static` | 6.703 | 1.201 | 0.365 | 0.196 |
| `dynamic,1` | 6.703 | 0.432 | 0.119 | **0.068** |
| `dynamic,100` | 6.702 | 0.564 | 0.466 | 0.466 |
| `guided` | 6.701 | 0.548 | 0.152 | 0.086 |

### Speedup and efficiency — `schedule(dynamic, 1)` (the winner)

T(1) reference = 6.703 s

| Threads | T(P) s | Speedup | Efficiency |
|---|---|---|---|
| 1 | 6.703 | 1.00 | 1.00 |
| 16 | 0.432 | 15.51 | 0.97 |
| 64 | 0.119 | 56.33 | 0.88 |
| 128 | 0.068 | 98.57 | 0.77 |

---

## 2. `tasks` — Reference Sweep (`grainsize=1` on outer loop)

| Threads | T(P) s | Speedup | Efficiency |
|---|---|---|---|
| 1 | 6.559 | 1.00 | 1.00 |
| 16 | 0.576 | 11.39 | 0.71 |
| 64 | 0.455 | 14.42 | 0.23 |
| 128 | 0.458 | 14.32 | 0.11 |

---

## 3. Winner Analysis

### Clear winner: `schedule(dynamic, 1)` for `parallel_for`

At **128 threads**: `dynamic,1` = **0.068 s** vs next-best (`guided`) = 0.086 s — **21 % faster**.

#### Why each schedule behaves as it does

| Schedule | Chunks at P=128 | Behaviour |
|---|---|---|
| `static` | 128 equal strips | Some strips hit high-cost boundary rows; fast strips finish and idle. Speedup degrades. |
| `dynamic,1` | 5000 chunks (one row each) | 5000 ≫ 128 → every thread stays busy; fine enough granularity to absorb per-row cost variance. |
| `dynamic,100` | 50 chunks (100 rows each) | 50 < 128 → 78 threads receive no work at all → starvation. Flat scaling beyond 16T. |
| `guided` | decreasing from ~39 rows down to 1 | Good balance but coarser initial chunks mean last few tasks can still cause tail latency. |

#### Why tasks plateau at 64–128 threads

The `taskloop grainsize(1)` applies to the **outer** loop (`i0 ∈ 0..50`), creating **50 tasks** each handling 25 j-tiles sequentially.  
With 128 threads and only 50 tasks, 78 threads are permanently idle → efficiency collapses to ~0.11.

#### Head-to-head at 128T: best `parallel_for` vs `tasks`

| Variant | T(128) s | Speedup vs serial |
|---|---|---|
| `parallel_for schedule(dynamic,1)` | **0.068** | 98.6 × |
| `tasks (grainsize=1 outer)` | 0.458 | 14.3 × |

**`parallel_for` wins by 6.7 ×** at 128 threads.

---

## 4. Decision

**Hardcode `schedule(dynamic, 1)` in `mandelbrot_for.cpp`.**

This is already in place:
```cpp
#pragma omp parallel for reduction(+ : outside) default(none) shared(J_HALF) \
    schedule(dynamic, 1)
```

Rationale: 5000 outer rows / 1-row chunks = 5000 work items for 128 threads. Near-perfect
load balance (efficiency 77 %) despite the ~100 × per-pixel cost variation near the
Mandelbrot boundary. `dynamic,100` and `guided` both under-saturate at extreme thread counts
due to insufficient chunk count.

---

## 5. Files

| File | Contents |
|---|---|
| `sweep-for-static-{1,16,64,128}.json` | `schedule(static)` hyperfine data |
| `sweep-for-dynamic_1-{1,16,64,128}.json` | `schedule(dynamic,1)` hyperfine data |
| `sweep-for-dynamic_100-{1,16,64,128}.json` | `schedule(dynamic,100)` hyperfine data |
| `sweep-for-guided-{1,16,64,128}.json` | `schedule(guided)` hyperfine data |
| `sweep-tasks-gs1-{1,16,64,128}.json` | `tasks grainsize=1` hyperfine data |
| `logs/a2_sweep.o2597440` | PBS job stdout — human-readable summary of all runs |

---

## Next step

Run `qsub evaluate.pbs` on CX3 to produce `perf-results-a2.json` (formal Phase 4 benchmark).
The formal run uses the already-hardcoded `schedule(dynamic, 1)`.
