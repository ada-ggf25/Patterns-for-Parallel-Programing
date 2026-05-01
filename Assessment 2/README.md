# Assignment 2 — Mandelbrot: two-variant comparison

This is the **starter repo** for Assignment 2 of the PPP-OpenMP assessment. **Target: 30 marks** out of 100 (A1=20, A2=30, A3=40, plus a cohort-wide 10-pt hygiene bucket).

Estimate the area of the Mandelbrot set on a 5000×5000 grid — once with `#pragma omp parallel for`, once with `#pragma omp task` / `taskloop`, and justify which variant you'd ship.

## Why two variants?

Per-pixel cost near the Mandelbrot boundary varies by ~100×. A static `parallel for` leaves some threads idle while others chew through the hard regions. Task-parallelism (over tiles) gives dynamic load balance. Your job is to *measure* the gap and *justify* the recommendation.

## What you have

- `mandelbrot_serial.cpp` — reference serial **(do not edit)**.
- `mandelbrot_for.cpp` — your parallel-for implementation.
- `mandelbrot_tasks.cpp` — your task-parallel implementation.
- `mandelbrot.h` — shared grid size, tile size, declarations.
- `expected_output.txt` — reference `outside` count + area.
- `questions.md`, `answers.csv`, `REFLECTION.md`, `tables.csv`, `CHOICE.md` — deliverables.
- `evaluate.pbs` — Rome perf harness (run with `qsub evaluate.pbs` from your CX3 account).

## Build (laptop)

```bash
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18    # any Clang ≥ 16 will do
cmake --build build -j
OMP_NUM_THREADS=4 ./build/mandelbrot_for
OMP_NUM_THREADS=4 ./build/mandelbrot_tasks
```

Recommended toolchain:
- macOS: `brew install llvm libomp cmake ninja`; use `/opt/homebrew/opt/llvm/bin/clang++`.
- Ubuntu / WSL: `apt install clang-18 libomp-18-dev cmake ninja-build`.

## What to do

1. Implement both variants. CI builds both targets and times each.
2. Fill in `tables.csv` — two rows per thread count (one per variant), with measured times, speedups, and efficiencies.
3. Fill in `CHOICE.md`'s **structured header** (`recommended:`, `measured_tasks_s:`, `measured_for_s:`, optionally `justification_keyword:`). The grader parses it deterministically.
4. Answer `questions.md` in `answers.csv`.
5. Fill in `REFLECTION.md`.

## Sanity-check under TSan locally before pushing

```bash
clang++ -fopenmp -fsanitize=thread -g -O1 \
    mandelbrot_for.cpp mandelbrot_serial.cpp -o mandelbrot_for_tsan
OMP_NUM_THREADS=4 ./mandelbrot_for_tsan
```

(Repeat with `mandelbrot_tasks.cpp`.)

## Rubric (30 pts)

| Component | Pts | How measured |
|---|---|---|
| Build + TSan clean (both variants) | 3 | Clang-18 + TSan + Archer |
| Correctness (graduated, per variant per thread count) | 6 | `smart_diff.py` |
| Reference-parallel-time perf of the **better variant** | 8 | `min(1.0, T_ref(128) / min(T_for, T_tasks)) × 8` against the published `T_ref` measured on Rome; correctness-gated |
| CHOICE.md evidence consistency | 4 | Deterministic parse. Recommendation must be either (a) the variant your own committed `perf-results-a2.json` shows as faster, OR (b) the slower variant with a defensible keyword. **Either variant can be the right answer.** |
| `tables.csv` internal consistency | 2 | Per-row `speedup = T(1)/T(P)` and `efficiency = speedup/P` within 2 % (no canonical cross-check) |
| Style | 2 | Lint |
| MCQ | 2 | Deterministic auto-grading |
| REFLECTION.md format + completion | 1 | CI-format-check (no canonical numerical cross-check) |
| Reasoning question (instructor-marked) | 2 | Manual 0/1/2 |

## Hygiene (10 pts, cohort-wide)

A separate **10-pt Hygiene bucket** — build cleanliness, lint compliance (clang-format / clang-tidy / cppcheck), README / English readability — is graded once across A1+A2+A3 on your final state. Hygiene in *this* repo contributes. See `assessment/rubric.md` in the lectures repo for the breakdown.

## CHOICE.md format

```
---
recommended: tasks        # one of: tasks | parallel_for
measured_tasks_s: 4.21
measured_for_s: 5.03
justification_keyword: irregular_load_balance    # optional — see list below
---
```

Defensible keywords (required if you recommend the variant your own data shows as *slower*):

- `irregular_load_balance`
- `scales_better_at_128T`
- `simpler_to_maintain`
- `future_proof_for_dynamic_work`

## Grading model

Grading is run once by the instructor at the **end of day 5** on a canonical CX3 Rome node. **All performance is correctness-gated** — any thread-count correctness fail zeros the perf component. No LLM is used in the summative grading path.

## What the formative CI checks (every push)

- **Build + correctness** of both variants at a small spread of thread counts (`{1, 2, 4, 8, 16}`) under Clang-18 + ThreadSanitizer + Archer OMPT.
- **Lint**: clang-format, clang-tidy, cppcheck.
- **REFLECTION format**: required headers present + each section ≥ 50 words. (Format only.)
- **Language check**: only English in Markdown and C++ comments.
- **No committed build artifacts**: `.o` / executables / `build/` are rejected.

There is **no Rome benchmark CI on student forks**. Performance is measured by the instructor at the end of the cohort on a CX3 Rome node. You can run `evaluate.pbs` on your own CX3 account to populate your `tables.csv` and your `perf-results-a2.json` evidence base for `CHOICE.md`.

## What you may NOT do

- Do not rename source files or public function signatures.
- Do not add new headers / dependencies / third-party libraries.
- Do not modify `.github/workflows/` (overwritten at grading time).
- Do not touch `bin/smart_diff.py` or other harness files.

If you need to change something not covered above, ask first.

## Useful pointers

- Lectures repo (slides, snippets, brief, rubric): https://github.com/ese-ada-lovelace-2025/ppp-openmp
- OpenMP 5.1 spec: https://www.openmp.org/spec-html/5.1/openmp.html
- Imperial CX3 docs: https://imperialcollegelondon.atlassian.net/wiki/spaces/HPC/

## Assessment timeline

Brief released day 2 morning. A2 is completable by **end of day 3**. Day 5 final snapshot is graded.
