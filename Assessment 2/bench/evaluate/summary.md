# Formal Benchmark — Phase 4 (`evaluate.pbs`)

**Job:** `a2_mandelbrot.o2598408` (PBS job ID 2598408)  
**Hardware:** CX3 Rome — AMD EPYC 7742, 128 physical cores, dual-socket, SMT off  
**Build:** GCC 14.3.0 (requested 13.3.0; 14.3.0 was the available version), `-O3 -march=znver2 -mavx2 -fopenmp`  
**Harness:** `hyperfine --warmup 1 --min-runs 3` (more runs at higher thread counts)  
**Grid:** 5000 × 5000 (upper half only, ×2 symmetry) = 5000 × 2500 iterations  
**Log:** `logs/a2_mandelbrot.o2598408`

---

## Raw min times (from hyperfine JSON)

All times are the **minimum** wall-clock value from the hyperfine JSON (most stable run).

| Variant | T(1) s | T(16) s | T(64) s | T(128) s |
|---|---|---|---|---|
| `parallel_for` (`schedule(dynamic,1)`) | 6.5982 | 0.4221 | 0.1144 | **0.0659** |
| `tasks` (`taskloop grainsize(1)`) | 6.4334 | 0.5627 | 0.4409 | 0.4430 |

---

## Speedup and efficiency

### `parallel_for schedule(dynamic, 1)`

T(1) reference = 6.5982 s

| Threads | T(P) s | Speedup | Efficiency |
|---|---|---|---|
| 1 | 6.5982 | 1.00 | 1.00 |
| 16 | 0.4221 | 15.63 | 0.98 |
| 64 | 0.1144 | 57.68 | 0.90 |
| 128 | 0.0659 | 100.12 | 0.78 |

Near-linear scaling to 128T. Efficiency holds above 0.78 even at full socket.

### `tasks (taskloop grainsize=1 outer loop)`

T(1) reference = 6.4334 s

| Threads | T(P) s | Speedup | Efficiency |
|---|---|---|---|
| 1 | 6.4334 | 1.00 | 1.00 |
| 16 | 0.5627 | 11.43 | 0.71 |
| 64 | 0.4409 | 14.59 | 0.23 |
| 128 | 0.4430 | 14.52 | 0.11 |

Tasks plateau after 16T. Root cause: `taskloop grainsize(1)` on 50 outer iterations = 50 tasks.
With 128 threads, 78 threads receive no work → efficiency collapses to 0.11.

---

## Winner at 128T

| Variant | T(128) s | Speedup vs serial |
|---|---|---|
| `parallel_for schedule(dynamic,1)` | **0.0659** | 100.1 × |
| `tasks (grainsize=1 outer)` | 0.4430 | 14.5 × |

**`parallel_for` wins by 6.72 ×** at 128 threads. Consistent with Phase 3.5 sweep prediction (0.068 s).

---

## Consistency with Phase 3.5 sweep

| Metric | Sweep (3.5) | Formal (4) | Δ |
|---|---|---|---|
| `parallel_for` T(128) | 0.068 s | 0.0659 s | −3 % |
| `tasks` T(128) | 0.458 s | 0.4430 s | −3 % |

Results agree within 3 % — expected given different job IDs and node allocation times.

---

## Files

| File | Contents |
|---|---|
| `mandelbrot-parallel_for-{1,16,64,128}.json` | `parallel_for` hyperfine data at each thread count |
| `mandelbrot-tasks-{1,16,64,128}.json` | `tasks` hyperfine data at each thread count |
| `perf-results-a2.json` | Combined summary — **this file is committed at repo root** for the grader |
| `logs/a2_mandelbrot.o2598408` | PBS job stdout |
