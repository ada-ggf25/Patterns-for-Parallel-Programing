# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this directory is

Local benchmark data from CX3 Rome HPC runs for PPP-OpenMP Assessment 2 (Mandelbrot). **This directory is not committed to git** — raw JSON and PBS logs are local analysis only. The sole exception is `perf-results-a2.json`, which must be committed at the **repo root** (not here) for the grader.

## Extracting times from hyperfine JSON

```bash
python3 ../bin/hyperfine_min_time.py <file>.json
```

All reported times use the `min` value (not mean) from hyperfine output — most stable run.

## Directory layout

```
bench/
  sweep/     Phase 3.5 — schedule exploration (4 schedules × 4 thread counts)
  evaluate/  Phase 4 — formal benchmark (both variants × 4 thread counts)
```

Each subdirectory has a `summary.md` with human-readable analysis and a `logs/` folder with the PBS job stdout.

## Consistency rule for tables.csv

When filling `../tables.csv` from these results:
- `speedup(P) = T(1) / T(P)`
- `efficiency(P) = speedup(P) / P`
- Both must match the reported values to within **2 %** (grader check)

## Key results

| Phase | Job | Winner |
|---|---|---|
| 3.5 sweep | `a2_sweep.o2597440` | `parallel_for schedule(dynamic,1)` — 0.068 s at 128T |
| 4 formal | `a2_mandelbrot.o2598408` | `parallel_for` — 0.0659 s vs tasks 0.4430 s (6.72 × faster) |

See `sweep/summary.md` and `evaluate/summary.md` for full analysis.
