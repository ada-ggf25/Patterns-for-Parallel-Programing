# bench/

Local benchmark data collected from CX3 Rome runs. **Do not commit** raw JSON or PBS log files — only `perf-results-a2.json` is committed (at repo root, not here).

## Directory structure

```
bench/
  sweep/            Phase 3.5 — schedule-sweep results (OMP_SCHEDULE runtime run)
    summary.md      Human-readable analysis and winner decision
    sweep-for-*.json        parallel_for schedule variants (4 schedules × 4 thread counts)
    sweep-tasks-gs1-*.json  tasks grainsize=1 reference (4 thread counts)
    logs/
      a2_sweep.o2597440     PBS job stdout from the sweep run
  evaluate/         Phase 4 — formal benchmark via evaluate.pbs
    summary.md      Human-readable analysis, speedup tables, winner decision
    mandelbrot-parallel_for-{1,16,64,128}.json   parallel_for hyperfine data
    mandelbrot-tasks-{1,16,64,128}.json          tasks hyperfine data
    perf-results-a2.json    Combined summary (also committed at repo root for grader)
    logs/
      a2_mandelbrot.o2598408  PBS job stdout from the evaluate run
```

## Run history

| Phase | Job | Date | Purpose |
|---|---|---|---|
| 3.5 sweep | `a2_sweep.o2597440` | 2026-04 | Schedule selection for `parallel_for` |
| 4 formal | `a2_mandelbrot.o2598408` | 2026-04 | Formal benchmark for tables.csv / CHOICE.md |

## Results at a glance

### Phase 3.5 sweep — winner: `schedule(dynamic, 1)`

| Schedule | T(128) s | Notes |
|---|---|---|
| `dynamic,1` | **0.068** | Winner — 5000 chunks saturate 128T |
| `guided` | 0.086 | Good but coarser initial chunks |
| `static` | 0.196 | Boundary rows cause hot-spots |
| `dynamic,100` | 0.466 | Only 50 chunks → thread starvation |

See `sweep/summary.md` for the full analysis.

### Phase 4 formal — winner: `parallel_for`

| Variant | T(1) s | T(128) s | Speedup@128T | Efficiency@128T |
|---|---|---|---|---|
| `parallel_for` | 6.5982 | **0.0659** | 100.1 × | 0.78 |
| `tasks` | 6.4334 | 0.4430 | 14.5 × | 0.11 |

**`parallel_for` is 6.72 × faster than `tasks` at 128T.**  
See `evaluate/summary.md` for the full analysis.
