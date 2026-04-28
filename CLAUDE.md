# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

Study and teaching materials for the **Patterns for Parallel Programming** module at Imperial College London (MSc Applied Computational Science and Engineering). The module runs over three weeks and covers HPC cluster usage, OpenMP shared-memory programming, and MPI distributed-memory programming.

## Repository structure

```
Lecture 1/          Intro to HPC on CX3: modules, PBS, OpenMP + MPI overview
Lecture 2/          OpenMP in depth: days 2–4 (parallel-for, tasks, Jacobi stencil)
                    Also contains student assessment (A1–A3) and grading rubric
obsidian_vault/     Student study notes (Obsidian markdown) — mirrors Lecture 1 content only
```

Each subdirectory has its own `CLAUDE.md` with build commands, architecture, and CI details specific to that lecture. **Always read the relevant subdirectory CLAUDE.md before making changes inside it.**

## Branch naming

Student branches follow the pattern `<initials><year>` (e.g. `ggf25`). The `main` branch holds the canonical instructor version. CI runs on push/PR to `main`.

## High-level architecture

Both lectures follow the same pattern:
- **Quarto (`.qmd`) → RevealJS HTML / Beamer PDF** for slides, built with `make html` / `make pdf`.
- **C++ snippets / examples** that are both compiled+tested and embedded in slides. Source lives in `snippets/` (Lecture 2) or `examples/` (Lecture 1); never edit auto-generated `_partials/` directly.
- **CX3 cluster** (PBS scheduler, AMD Rome / Intel Ice Lake nodes) is the target execution environment. PBS job scripts reference build outputs relative to the submit CWD (`examples/` in Lecture 1).

## Common cross-cutting notes

- Module stack on CX3: `ml tools/prod GCC OpenMPI CMake`
- PBS jobs: submit from `examples/` (Lecture 1) so relative paths in `.pbs` scripts resolve correctly.
- Slides and tested code are kept in sync via a partials pipeline — always edit the `.cpp` source, never the generated `.qmd` partials.
- `obsidian_vault/` is read-only study notes — no build artefacts; never add front-matter or YAML headers to notes there.
- `Lecture 1/AGENTS.md` is an identical copy of `Lecture 1/CLAUDE.md` kept for Codex compatibility; keep the two in sync if either is updated.
