# CLAUDE.md — bench/

This directory holds all CX3 Rome benchmark artefacts for Assessment 3.
Do **not** commit any compiled binaries or `build/` directories here.

---

## Directory structure

```
bench/
├── perf-results-a3.json          # Summary: min wall-clock time per configuration (committed to repo)
├── a3_stencil.o2619317           # PBS job stdout/stderr (job ID 2619317, wall 00:05:44)
└── raw/
    ├── stencil-1.json            # Full hyperfine output — core @ 1T
    ├── stencil-16.json           # Full hyperfine output — core @ 16T
    ├── stencil-64.json           # Full hyperfine output — core @ 64T
    ├── stencil-128.json          # Full hyperfine output — core @ 128T
    ├── ext_numa_first_touch_stencil_ft-128.json     # Extension (after) @ 128T
    └── ext_numa_first_touch_stencil_naive-128.json  # Extension (before) @ 128T
```

---

## Benchmark environment

- **Node:** CX3 Rome — dual AMD EPYC 7742 (128 cores total, 8 NUMA domains, NPS4)
- **Compiler:** GCC 14.3.0, flags `-O3 -march=znver2 -mavx2`
- **OpenMP:** version 4.5, `OMP_PROC_BIND=close OMP_PLACES=cores`
- **Method:** `hyperfine --warmup 1 --min-runs 3`; `perf-results-a3.json` records the **minimum** wall-clock time of each 3-run group
- **PBS script:** `evaluate.pbs` in repo root

---

## Key results (min times)

| Configuration | Threads | Min time (s) |
|---|---|---|
| core | 1 | 40.8655 |
| core | 16 | 14.1121 |
| core | 64 | 3.6213 |
| core | 128 | 1.9107 |
| ext_numa_first_touch_stencil_ft | 128 | 1.9067 |
| ext_numa_first_touch_stencil_naive | 128 | 18.7130 |

---

## Derived quantities

**Bandwidth formula** (professor-specified, 56 B per interior-point update = 6 reads × 8 B + 1 write × 8 B):

```
bytes_moved = (NX-2) × (NY-2) × (NZ-2) × NSTEPS × 56
            = 510³ × 100 × 56
            = 742,845,600,000 B  →  742.846 GB

BW_GBs = 742.846 / time_s
```

**Speedup:** `S(P) = T(1, core) / T(P)` for all configurations including the extension row.

**Efficiency:** `E(P) = S(P) / P`

**Roofline fraction:**

| Thread count | STREAM reference | Formula |
|---|---|---|
| 1 | 231.5 GB/s | `BW / 231.5` |
| 16 | 116.0 GB/s | `BW / 116.0` |
| 64 | 116.0 GB/s | `BW / 116.0` |
| 128 | 231.5 GB/s | `BW / 231.5` |

STREAM references: 231.5 GB/s at 128T full-node, 116.0 GB/s at 64T one-socket close-binding.

**Computed table** (see `tables.csv` in repo root for the submission copy):

| Threads | Stage | T (s) | Speedup | Efficiency | BW (GB/s) | Roofline |
|---|---|---|---|---|---|---|
| 1 | core | 40.865 | 1.000 | 1.000 | 18.18 | 0.079 |
| 16 | core | 14.112 | 2.896 | 0.181 | 52.64 | 0.454 |
| 64 | core | 3.621 | 11.285 | 0.176 | 205.1 | 1.768 |
| 128 | core | 1.911 | 21.39 | 0.167 | 388.8 | 1.680 |
| 128 | ext (ft) | 1.907 | 21.43 | 0.167 | 389.6 | 1.683 |

> Roofline > 1.0 at 64T and 128T is expected: the stride-1 inner loop (k-dimension) gives cache-line
> reuse for the k±1 stencil neighbours, so effective memory throughput exceeds the STREAM-Triad
> roofline. The stencil is bandwidth-bound but benefits from spatial locality.

**Extension delta:**

```
before_time_s  = 18.713 s  (stencil_naive — serial init, all pages on socket 0)
after_time_s   = 1.907 s   (stencil_ft   — parallel first-touch, pages near each thread)
delta_percent  = (18.713 - 1.907) / 18.713 × 100 = 89.81%
speedup        = 18.713 / 1.907 = 9.81×
```

Threshold for full marks: ≥ 15% → **PASS** (89.81% >> 15%).

---

## File format: perf-results-a3.json

The `evaluate.pbs` script writes one JSON object with a `results` array.
Each entry has at minimum:

```json
{
  "thread_count": <int>,
  "stage": "<core | ext_<name>>",
  "time_s": <float>          // minimum wall-clock time across hyperfine runs
}
```

Extension entries additionally carry `"binary": "<target_name>"` to distinguish the
before/after variants within the same stage.

## File format: raw/*.json

Standard `hyperfine --export-json` output. Fields relevant here:

- `results[0].min` — minimum wall time (seconds); used in `perf-results-a3.json`
- `results[0].mean`, `results[0].stddev` — for σ reporting
- `results[0].times` — all per-run wall times (3 values at `--min-runs 3`)
- `results[0].memory_usage_byte` — peak RSS per run; confirms 2 × 512³ × 8 B ≈ 2.05 GB working set

---

## Related

- `../perf-results-a3.json` — symlinked/copied summary at repo root (grader reads this)
- `../tables.csv` — submission table computed from these results
- `../EXTENSION.md` — before/after times derived from `ext_numa_first_touch_stencil_{naive,ft}-128.json`
- `../obsidian_vault/assessment/A3 Benchmark Results.md` — analysis and interpretation
- `../obsidian_vault/assessment/A3 Progress.md` — Phase 4 checklist and context
