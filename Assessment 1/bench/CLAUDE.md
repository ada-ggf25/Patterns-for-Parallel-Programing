# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this directory is

Benchmark results from a CX3 Rome PBS job (`evaluate_all.pbs` at the repo root). The job swept three OpenMP schedules — `static`, `dynamic,64`, `guided` — at thread counts `{1, 16, 64, 128}` on a dedicated 128-core AMD EPYC 7742 node. All times are `min` values from hyperfine.

## Structure

```
bench/
  static/          integrate-{1,16,64,128}.json   # schedule(static)
  dynamic_64/      integrate-{1,16,64,128}.json   # schedule(dynamic,64)
  guided/          integrate-{1,16,64,128}.json   # schedule(guided)
  *.o<jobid>       PBS stdout+stderr log (build output + human-readable summary)
```

The subdirectory for `dynamic,64` uses an underscore (`dynamic_64`) because the PBS script replaces commas with underscores via `${sched//,/_}` for filesystem safety.

## Extracting times

```bash
# Min time from a single JSON (the value to use in tables.csv):
python3 bin/hyperfine_min_time.py bench/guided/integrate-128.json

# Quick comparison across all schedules at a given thread count:
for s in static dynamic_64 guided; do
    echo -n "$s T(128): "
    python3 bin/hyperfine_min_time.py bench/$s/integrate-128.json
done
```

Always use `min` (not `mean`) — `hyperfine`'s `mean` includes outlier runs caused by OS scheduling noise.

## Regenerating data

Regeneration requires:
1. `integrate.cpp` must use `schedule(runtime)` — the PBS script sets `OMP_SCHEDULE` to select the schedule at runtime.
2. A CX3 login node:

```bash
ssh ggf25@login.cx3.hpc.ic.ac.uk
cd ppp-openmp-assessment-1-ada-ggf25
qsub evaluate_all.pbs
qstat -u ggf25   # monitor
```

Results overwrite `bench/<schedule>/integrate-<p>.json`. The PBS output log lands in the repo root as `a1_integrate_all_schedules.o<jobid>` and should be moved into `bench/` for archival.

## Results summary (2026-04-29 run, job 2593047)

**Winner: `schedule(guided)` at 128T** — T(128) = 0.0446 s, speedup 42.49×.

| Schedule | T(1) s | T(16) s | T(64) s | T(128) s |
|---|---|---|---|---|
| `static` | 1.8939 | 0.3376 | 0.0961 | 0.0566 |
| `dynamic,64` | 1.9048 | 0.1401 | 0.0697 | 0.0987 |
| `guided` | 1.8961 | 0.2297 | 0.0719 | 0.0446 |

`dynamic,64` regresses at 128T (worse than 64T): chunk=64 over N=1e8 iterations produces ~1.5 M dispatch events, saturating the atomic work-queue at 128 threads. `guided` avoids this by tapering chunk sizes.

Full analysis: `../obsidian_vault/assessment/A1 Benchmark Results.md`.
