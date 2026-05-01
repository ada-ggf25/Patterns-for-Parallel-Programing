# PPP — Vault Index

This vault covers the full **Patterns for Parallel Programming** module at Imperial College London (Introduction: HPC intro + Week 1: OpenMP in depth + assessments).

---

## Assessment 1 — COMPLETE ✅

**Status: COMPLETE — all phases done, all CI green** | Branch: `ggf25` | 20 pts

→ [[assessment/A1 Progress]] — completed checklist and scoring tracker
→ [[assessment/A1 Integration]] — full technical reference (kernel, schedule theory, REFLECTION guide)
→ [[assessment/A1 Benchmark Results]] — CX3 Rome measured times (guided wins at 128T: 0.0446 s, 42.49× speedup)

---

## Assessment 2 — COMPLETE ✅

**Status: COMPLETE — all 8 phases done, all 5 CI jobs green, submitted for instructor marking** | Branch: `ggf25` | 30 pts

→ [[assessment/A2 Progress]] — completed checklist and scoring tracker
→ [[assessment/A2 Mandelbrot]] — full technical reference (kernel, implementation patterns, CHOICE.md format, REFLECTION guide)
→ [[assessment/A2 Benchmark Results]] — full Phase 3.5 sweep + Phase 4 formal data

**Phase overview:**

| Phase | Task | Status |
|---|---|---|
| 1 | Implement `mandelbrot_parallel_for()` in `mandelbrot_for.cpp` | ✅ `schedule(dynamic,1)` hardcoded |
| 2 | Implement `mandelbrot_tasks()` in `mandelbrot_tasks.cpp` | ✅ `taskloop grainsize(1)` + reduction |
| 3 | TSan check locally + push for CI | ✅ CI green (Build & TSan correctness) |
| 3.5 | Schedule sweep (`sweep.pbs`) + hardcode winner | ✅ `dynamic,1` = 0.068 s at 128T — winner |
| 4 | CX3 formal benchmark (`qsub evaluate.pbs`) + fill `tables.csv` | ✅ `parallel_for` 0.0659 s, `tasks` 0.4430 s at 128T |
| 5 | Answer 15 MCQs in `answers.csv` | ✅ All 15 answered |
| 6 | Fill `CHOICE.md` header with measured evidence | ✅ `parallel_for` recommended; values from `perf-results-a2.json` |
| 7 | Write all sections of `REFLECTION.md` | ✅ All sections + reasoning question committed |
| 8 | Lint clean + all CI jobs green | ✅ All 5 CI jobs green |

**Result:** `parallel_for schedule(dynamic,1)` = **0.0659 s** at 128T (100× speedup, 0.78 efficiency) vs `tasks` = 0.4430 s (14.5×) — `parallel_for` wins by 6.7×.

---

## Assessment 3 — IN PROGRESS 🔄

**Status: IN PROGRESS** | Branch: `ggf25` | 40 pts (25 core + 15 extension)

→ [[assessment/A3 Progress]] — phase-by-phase implementation guide and scoring tracker **(start here)**
→ [[assessment/A3 Jacobi]] — full technical reference (kernel, extensions, roofline, deliverables format)
→ [[assessment/A3 MCQ]] — all 15 MCQ questions with correct answers and rationale
→ [[assessment/A3 Benchmark Results]] — CX3 Rome measured times (fill after Phase 4)

**Phase overview:**

| Phase | Task | Status |
|---|---|---|
| 1 | Parallelise `jacobi_step()` in `core/stencil.cpp` with `collapse(3)` | ⬜ |
| 2 | Pick ONE extension; implement before + after variants in `extension/<branch>/` | ⬜ |
| 3 | TSan check locally + push for CI | ⬜ |
| 4 | CX3 benchmark (`qsub evaluate.pbs`) + fill `tables.csv` + `perf-results-a3.json` | ⬜ |
| 5 | Answer 15 MCQs in `answers.csv` | ⬜ |
| 6 | Fill `EXTENSION.md` header with measured before/after/delta | ⬜ |
| 7 | Write all 5 sections of `REFLECTION.md` | ⬜ |
| 8 | Lint clean + all CI jobs green | ⬜ |

**Kernel:** 7-point 3D Jacobi stencil, OI ≈ 0.14 FLOPs/byte, DRAM-bandwidth-bound. Roofline target: ≥ 0.50 of 231.5 GB/s STREAM at 128T on Rome.

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
33. [[assessment/Pulling Template Updates]] — how to sync template fixes into your fork (file-checkout, not rebase).
34. [[assessment/A1 Progress]] — ✅ A1 complete.
35. [[assessment/A1 Integration]] — 20 pts, full technical reference for schedule sweep.
36. [[assessment/A2 Progress]] — ✅ A2 complete.
37. [[assessment/A2 Mandelbrot]] — 30 pts, parallel-for vs tasks, CHOICE.md format, implementation patterns.
38. [[assessment/A3 Jacobi]] — 40 pts, bandwidth-bound stencil + extension.
39. [[assessment/A3 Progress]] — ⬜ A3 in progress (start here for implementation).
40. [[assessment/A3 MCQ]] — MCQ answers and rationale.
41. [[assessment/A3 Benchmark Results]] — CX3 Rome times (fill after Phase 4).

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
