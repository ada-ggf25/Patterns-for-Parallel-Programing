# A3 Benchmark Results

CX3 Rome benchmark data. Populate after `qsub evaluate.pbs` completes.

**Status:** ⬜ Pending CX3 run

---

## Core: stencil at `{1, 16, 64, 128}` threads

| Threads | Stage | T(s) | Speedup | Efficiency | BW (GB/s) | Roofline fraction |
|---|---|---|---|---|---|---|
| 1 | core | — | 1.00 | 1.00 | — | — |
| 16 | core | — | — | — | — | — |
| 64 | core | — | — | — | — | — |
| 128 | core | — | — | — | — | — |
| 128 | extension | — | — | — | — | — |

STREAM reference: 231.5 GB/s at 128T, 116.0 GB/s at 64T (one socket).

Bandwidth formula: `BW ≈ 663 / T_s  [GB/s]`  (= FLOPs_total / OI / T; 7 × 510³ × 100 FLOPs ÷ 0.14 OI)

Roofline fraction: `BW / 231.5` at 128T (graded threshold).

Roofline scoring:
| Fraction | Points |
|---|---|
| ≥ 0.70 | 6 / 6 |
| ≥ 0.50 | 4.2 / 6 |
| ≥ 0.30 | 2.4 / 6 |
| ≥ 0.15 | 0.9 / 6 |
| < 0.15 | 0 |

---

## Extension delta

| Metric | Value |
|---|---|
| Extension chosen | — |
| before_time_s | — |
| after_time_s | — |
| delta_percent | — |
| Threshold (full marks) | ≥ 15% (NUMA/FS) or ≥ 1.2× (SIMD) |

---

## Raw output

Paste `perf-results-a3.json` content here after the CX3 run.

```json
```

---

## Related

- [[A3 Progress]] — Phase 4: benchmark submission and tables.csv.
- [[A3 Jacobi]] — scoring rubric and roofline thresholds.
- [[../performance/Roofline Model]] — how roofline fraction is computed.
