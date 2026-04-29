# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this directory is

Study notes for the **Patterns for Parallel Programming** module at Imperial College London, written in Obsidian-flavoured Markdown. The vault now covers the **full module** (Introduction + Week 1):

- **Introduction** (original): HPC intro, CX3, PBS, OpenMP basics, MPI basics.
- **Week 1** (added): OpenMP in depth (data sharing, synchronisation, tasks, SIMD, NUMA, roofline) + assessment notes (A1/A2/A3).

Companion code lives in `../Introduction/examples/` (Introduction) and `../Week 1/snippets/` (Week 1). These notes contain no build artefacts.

## Vault structure

```
Index.md         Entry point — full reading order for both weeks
cluster/         Hardware, login, filesystems, NUMA, topology (Introduction)
modules/         Lmod commands, tools/prod gateway, loading combos
pbs/             PBS directives, resource selection, job lifecycle, queues
openmp/          All OpenMP content — Introduction basics + Week 1 depth
  Introduction: OpenMP Overview, parallel/for/reduction directives,
                Variable Scoping, OMP Env Vars, Thread Pinning, Fork-Join,
                Amdahl's Law, Building OpenMP, Pitfalls, Reading the log
  Week 1: _OPENMP Macro, Data Races and TSan, Schedules,
          User-Defined Reductions, Barriers, critical and atomic,
          Memory Model, Locks, single and masked,
          Tasks, Task Dependences, taskloop
mpi/             SPMD model, six essential calls, reductions, hybrid mode
examples/        Walk-throughs of pi_serial / pi_openmp / pi_mpi
performance/     Week 1 performance topics
  Roofline Model, Performance Metrics, Timing omp_get_wtime,
  False Sharing, NUMA First Touch, SIMD, Loop Transformations,
  Six Sources of Overhead, STREAM and HPL
assessment/      A1 / A2 / A3 deliverables and scoring notes
  Assessment Overview
  A1 Progress     ← active checklist/roadmap for Assessment 1 (start here)
  A1 Integration  ← full technical reference for A1
  A2 Mandelbrot, A3 Jacobi
```

## Active assessment context

Assessment 1 (numerical integration, 20 pts) is currently in progress. The primary working files are:
- `../integrate.cpp` — parallelise `integrate_parallel()` here
- `../answers.csv` — fill MCQ answers (qid,answer format)
- `../tables.csv` — fill timing data from CX3 Rome benchmark
- `../REFLECTION.md` — write all four sections (≥ 50 words each)

Use `assessment/A1 Progress.md` as the ground truth for what is done and what is pending. Update the status checkboxes as phases complete. The branch is `ggf25`.

## Authoring conventions

- **Internal links** use Obsidian wiki-link syntax: `[[folder/Note Title]]` or `[[Note Title|display text]]`. The vault is configured for relative links (`newLinkFormat: relative`).
- **No front-matter** is used in any note — Obsidian properties are off; don't add YAML front-matter.
- Each note is self-contained but cross-links heavily. When adding a note, wire it into `Index.md` under the relevant topic map section and suggested reading order if it fits the linear flow.
- Code blocks use fenced triple-backtick with a language tag (`bash`, `cmake`, `cpp`, `text`).
- The `## Related` section at the bottom of most notes lists wiki-links to adjacent notes — maintain this pattern.

## Obsidian settings (`.obsidian/`)

Settings are checked in. Do not modify `app.json` or `core-plugins.json` without a reason — they control link format and enabled plugins for the whole vault.
