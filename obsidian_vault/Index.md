# PPP — Vault Index

This vault covers the full **Patterns for Parallel Programming** module at Imperial College London (Introduction: HPC intro + Week 1: OpenMP in depth + assessments).

---

## Assessment 1 — COMPLETE ✅

**Status: COMPLETE — all phases done, all CI green** | Branch: `ggf25` | 20 pts

→ [[assessment/A1 Progress]] — completed checklist and scoring tracker
→ [[assessment/A1 Integration]] — full technical reference (kernel, schedule theory, REFLECTION guide)
→ [[assessment/A1 Benchmark Results]] — CX3 Rome measured times (guided wins at 128T: 0.0446 s, 42.49× speedup)

---

## Assessment 2 — Active Work

**Status: IN PROGRESS — Phases 1, 2, 3 & 3.5 complete; next: Phase 4 (CX3 formal benchmark)** | Branch: `ggf25` | 30 pts

→ [[assessment/A2 Progress]] — step-by-step checklist, phases, and scoring tracker *(start here)*
→ [[assessment/A2 Mandelbrot]] — full technical reference (kernel, implementation patterns, CHOICE.md format, REFLECTION guide)
→ [[assessment/A2 Benchmark Results]] — Phase 3.5 sweep data filled; Phase 4 formal data pending

**Phase overview:**

| Phase | Task | Status |
|---|---|---|
| 1 | Implement `mandelbrot_parallel_for()` in `mandelbrot_for.cpp` | ✅ `schedule(dynamic,1)` hardcoded |
| 2 | Implement `mandelbrot_tasks()` in `mandelbrot_tasks.cpp` | ✅ `taskloop grainsize(1)` + reduction |
| 3 | TSan check locally + push for CI | ✅ CI green (Build & TSan correctness) |
| 3.5 | Schedule sweep (`sweep.pbs`) + hardcode winner | ✅ `dynamic,1` = 0.068 s at 128T — winner |
| 4 | CX3 formal benchmark (`qsub evaluate.pbs`) + fill `tables.csv` | ⬜ **NEXT** |
| 5 | Answer 15 MCQs in `answers.csv` (independent) | ⬜ |
| 6 | Fill `CHOICE.md` header with measured evidence from Phase 4 | ⬜ |
| 7 | Write all sections of `REFLECTION.md` | ⬜ |
| 8 | Lint clean + all CI jobs green | ⬜ (style + reflection CI still failing) |

**Critical path:** Phase 4 (CX3 formal benchmark) → Phases 6+7 (need `perf-results-a2.json`). Phase 5 is independent.

**Sweep winner:** `parallel_for schedule(dynamic,1)` = **0.068 s** at 128T vs `tasks` = 0.458 s — `parallel_for` wins by 6.7×.

**Key constraints:** do not edit `mandelbrot_serial.cpp`, `mandelbrot.h`, or `.github/workflows/`. Do not commit `build/`. **Do** commit `perf-results-a2.json` (required for CHOICE.md grading).

---

## Introduction — Suggested reading order

1. [[cluster/CX3 Overview]] — what the cluster is.
2. [[cluster/Login and Authentication]] — how to connect.
3. [[cluster/Filesystems]] — where your files live.
4. [[modules/Modules Overview]] → [[modules/Lmod Commands]] → [[modules/tools-prod gateway]] → [[modules/Loading Combos]].
5. [[pbs/PBS Overview]] → [[pbs/Job Script Anatomy]] → [[pbs/PBS Directives]] → [[pbs/Resource Selection]] → [[pbs/qsub qstat qdel]] → [[pbs/Log Files]].
6. [[examples/Pi Algorithm]] then [[examples/pi_serial]].
7. [[openmp/OpenMP Overview]] → [[openmp/parallel directive]] → [[openmp/for directive]] → [[openmp/reduction clause]] → [[openmp/Variable Scoping]] → [[openmp/Thread Pinning]] → [[examples/pi_openmp]].
8. [[mpi/MPI Overview]] → [[mpi/SPMD Model]] → [[mpi/MPI Six Essentials]] → [[mpi/MPI Reduce]] → [[examples/pi_mpi]].
9. [[mpi/OpenMP vs MPI]] and [[mpi/Hybrid MPI OpenMP]] to wrap up.

## Week 1 — Suggested reading order

**Day 2 — parallel regions, data sharing, work-sharing:**

10. [[openmp/_OPENMP Macro]] — version detection, team-query functions.
11. [[openmp/Data Races and TSan]] — races mechanism, TSan, `default(none)`.
12. [[openmp/Variable Scoping]] — full clause table including `default(none)` + `firstprivate` pitfall.
13. [[openmp/reduction clause]] → [[openmp/User-Defined Reductions]] — scalar and compound reductions.
14. [[openmp/Schedules]] — static / dynamic / guided, chunk tuning, sweep methodology.
15. [[performance/Roofline Model]] — OI formula, compute vs bandwidth regimes, Rome numbers.
16. [[performance/Performance Metrics]] — speedup, reference-parallel-time, roofline fraction.

**Day 3 — synchronisation, memory model, tasks:**

17. [[openmp/Barriers]] — implicit/explicit barriers, `nowait`, orphan-construct rule.
18. [[openmp/critical and atomic]] — critical sections, atomic ops, tool selection guide.
19. [[openmp/Memory Model]] — flush, acquire/release, visibility, publish/subscribe.
20. [[openmp/Locks]] — `omp_lock_t` lifecycle, RAII wrapper.
21. [[openmp/single and masked]] — one-shot execution, task spawn wrapper.
22. [[openmp/Tasks]] — lifecycle, recursive spawn, `taskwait`, `taskgroup`, data environment.
23. [[openmp/Task Dependences]] — `depend` clauses, DAG patterns.
24. [[openmp/taskloop]] — `grainsize`, tasks-vs-for guide.

**Day 4 — performance, NUMA, SIMD:**

25. [[performance/Six Sources of Overhead]] — Amdahl, imbalance, sync, scheduling, false sharing, HW.
26. [[performance/Timing omp_get_wtime]] — warm-up, min-of-k, `hyperfine`.
27. [[performance/False Sharing]] — MESI, cache-line padding, `alignas(64)`.
28. [[performance/NUMA First Touch]] — first-touch policy, parallel init, cross-socket cost.
29. [[performance/Loop Transformations]] — `collapse(N)`, tiling preview.
30. [[performance/SIMD]] — `omp simd`, `parallel for simd`, `declare simd`; `aligned`/`safelen` optional on Zen 2.
31. [[performance/STREAM and HPL]] — bandwidth and compute ceiling benchmarks.

**Assessments:**

32. [[assessment/Assessment Overview]] — 100-pt rubric, thread ladder, grading formulas.
33. [[assessment/A1 Progress]] — ✅ A1 complete.
34. [[assessment/A1 Integration]] — 20 pts, full technical reference for schedule sweep.
35. [[assessment/A2 Progress]] — ⬜ A2 checklist and progress tracker *(active)*.
36. [[assessment/A2 Mandelbrot]] — 30 pts, parallel-for vs tasks, CHOICE.md format, implementation patterns.
37. [[assessment/A3 Jacobi]] — 40 pts, bandwidth-bound stencil + extension.

## Map by topic

- **Cluster** — `cluster/`: hardware, login, filesystems, [[cluster/AMD Rome Architecture|Rome]] / [[cluster/Intel Ice Lake Architecture|Ice Lake]] internals, NUMA.
- **Modules** — `modules/`: Lmod cheat sheet, the [[modules/tools-prod gateway|tools/prod]] gateway.
- **PBS** — `pbs/`: directives, resource selection, queues, lifecycle, log files.
- **OpenMP** — `openmp/`: all directives, scoping, memory model, tasks, synchronisation.
- **Performance** — `performance/`: roofline, timing, false sharing, NUMA, SIMD, benchmarks.
- **MPI** — `mpi/`: SPMD, six essentials, reductions, hybrid mode.
- **Examples** — `examples/`: walk-throughs of `pi_serial`, `pi_openmp`, `pi_mpi`, NUMA latency.
- **Assessment** — `assessment/`: rubric, A1, A2, A3 deliverables and scoring.

## Quick-reference one-liners

```bash
ssh ggf25@login.cx3.hpc.ic.ac.uk           # connect
ml tools/prod GCC OpenMPI CMake            # full toolchain
qsub evaluate.pbs                          # submit A1 (run from repo root)
qstat -u $USER                             # monitor
OMP_NUM_THREADS=128 OMP_PROC_BIND=close OMP_PLACES=cores ./program
hyperfine --warmup 1 --runs 5 './program' --export-json perf.json
```
