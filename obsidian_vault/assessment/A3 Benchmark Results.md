# A3 Benchmark Results

CX3 Rome benchmark data from `qsub evaluate.pbs` (job 2619317, GCC 14.3.0, `-O3 -march=znver2 -mavx2`, `OMP_PROC_BIND=close OMP_PLACES=cores`).

**Status:** ✅ Complete — job 2619317 finished (wall 00:05:44, CPU 02:25:43)

---

## Core: stencil at `{1, 16, 64, 128}` threads

All times are the **minimum** of 3 hyperfine runs (--warmup 1 --min-runs 3).

| Threads | Stage | T (s) | Speedup | Efficiency | BW (GB/s) | Roofline fraction |
|---|---|---|---|---|---|---|
| 1 | core | 40.865 | 1.000 | 1.000 | 18.18 | 0.079 ¹ |
| 16 | core | 14.112 | 2.896 | 0.181 | 52.64 | 0.454 ² |
| 64 | core | 3.621 | 11.285 | 0.176 | 205.1 | 1.768 ² ³ |
| 128 | core | 1.911 | 21.39 | 0.167 | 388.8 | 1.680 ¹ |
| 128 | ext (ft) | 1.907 | 21.43 | 0.167 | 389.6 | 1.683 ¹ |

¹ Roofline denominator = 231.5 GB/s (full-node STREAM at 128T).  
² Roofline denominator = 116.0 GB/s (one-socket STREAM at 64T, close-binding).  
³ Roofline > 1.0 at 64T and 128T: the stencil inner loop (stride-1 in k) gives cache-line reuse for the k±1 neighbours, so effective bandwidth exceeds the STREAM-Triad roofline. This is consistent with the kernel being memory-bound but benefiting from spatial locality.

STREAM reference: 231.5 GB/s at 128T, 116.0 GB/s at 64T close-binding (one socket), 246.2 GB/s at 32T spread (1/CCX).

Bandwidth formula: `BW = 742.846 / T_s  [GB/s]`  
= (510³ × 100 × 56 B) / T_s / 1 × 10⁹  
= 742,845,600,000 B / T_s / 1 × 10⁹

Roofline fraction: `BW / 231.5` at 128T (graded threshold).

Roofline scoring:
| Fraction | Points |
|---|---|
| ≥ 0.70 | 6 / 6 |
| ≥ 0.50 | 4.2 / 6 |
| ≥ 0.30 | 2.4 / 6 |
| ≥ 0.15 | 0.9 / 6 |
| < 0.15 | 0 |

**Result: roofline fraction = 1.680 at 128T → 6/6 points (> 0.70 threshold).**

---

## Strong-scaling analysis

| Threads | Speedup | Notes |
|---|---|---|
| 1 → 16 | 2.90× | Below linear (16×): NUMA boundary between CCDs, OS pages not yet distributed across all NUMA domains |
| 16 → 64 | 3.89× (×11.3 cumulative) | Approaches one full socket; STREAM bandwidth scales well within socket 0 |
| 64 → 128 | 1.90× (×21.4 cumulative) | Crossing socket boundary via xGMI; inter-socket bandwidth gain is sub-linear |

Total speedup at 128T (core): **21.4×** out of a theoretical 128× maximum → **16.7% parallel efficiency**.  
Memory bandwidth is the dominant bottleneck: bandwidth saturates as more NUMA domains are added.

---

## Extension delta

| Metric | Value |
|---|---|
| Extension chosen | `numa_first_touch` |
| before_time_s | 18.713 s (stencil_naive, serial init, 128T) |
| after_time_s | 1.907 s (stencil_ft, parallel first-touch, 128T) |
| delta_percent | **89.81%** |
| Extension speedup | 9.81× (naive → ft) |
| Threshold (full marks) | ≥ 15% → **PASS (full 5/5)** |

**Interpretation:** Serial `init()` in `stencil_naive` faults all 2.1 GB of grid pages onto socket 0's NUMA domain. At 128T (64 threads per socket), all 64 threads on socket 1 must fetch every cache line via the xGMI inter-socket link (~3× latency vs local DRAM). Parallel first-touch in `stencil_ft` places each page on the NUMA domain of the thread that first writes it, matching the page placement to the jacobi_step traversal order. This removes the remote NUMA bottleneck entirely, recovering ~9.8× speedup (89.8% delta — well above the 15% full-marks threshold).

---

## Raw output

`perf-results-a3.json` (produced by evaluate.pbs, job 2619317):

```json
{"results": [
  {"thread_count": 1, "stage": "core", "time_s": 40.8654994291},
  {"thread_count": 16, "stage": "core", "time_s": 14.11213808506},
  {"thread_count": 64, "stage": "core", "time_s": 3.62134900326},
  {"thread_count": 128, "stage": "core", "time_s": 1.9106854628},
  {"thread_count": 128, "stage": "ext_numa_first_touch", "binary": "ext_numa_first_touch_stencil_ft", "time_s": 1.9066564296800002},
  {"thread_count": 128, "stage": "ext_numa_first_touch", "binary": "ext_numa_first_touch_stencil_naive", "time_s": 18.71295970616}]}
```

Full hyperfine output (mean ± σ, 3 runs each) from PBS stdout `a3_stencil.o2619317`:

| Configuration | Mean (s) | σ (s) | Min (s) | Max (s) |
|---|---|---|---|---|
| core @ 1T | 40.874 | 0.013 | 40.865 | 40.889 |
| core @ 16T | 14.121 | 0.008 | 14.112 | 14.127 |
| core @ 64T | 3.625 | 0.005 | 3.621 | 3.631 |
| core @ 128T | 1.922 | 0.018 | 1.911 | 1.944 |
| ext_ft @ 128T | 1.913 | 0.006 | 1.907 | 1.918 |
| ext_naive @ 128T | 18.747 | 0.054 | 18.713 | 18.810 |

PBS job metadata: wall 00:05:44, CPU 02:25:43, peak memory 2128764 KB ≈ 2.03 GB (matches 2 × 512³ × 8 B ≈ 2.05 GB expected working set).

---

## Related

- [[A3 Progress]] — Phase 4: benchmark submission and tables.csv.
- [[A3 Jacobi]] — scoring rubric and roofline thresholds.
- [[../performance/Roofline Model]] — how roofline fraction is computed.
