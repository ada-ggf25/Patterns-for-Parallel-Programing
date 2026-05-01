# Assignment 1 — Numerical integration

This is the **starter repo** for Assignment 1 of the PPP-OpenMP assessment. **Target: 20 marks** out of 100 (A1=20, A2=30, A3=40, plus a cohort-wide 10-pt hygiene bucket).

Parallelise a composite-trapezoid integration of a deliberately cost-non-uniform `f(x)` over `[0, 1]` using OpenMP.

## What you have

- `integrate_serial.cpp` — reference serial implementation **(do not edit)**. Contains `f(x)` with a spike region between `x ∈ [0.3, 0.4]` where per-evaluation cost is ~10× higher.
- `integrate.cpp` — your implementation goes here. Starter is a serial fallback.
- `integrate.h` — shared declarations.
- `expected_output.txt` — reference stdout for correctness checking.
- `questions.md`, `answers.csv`, `REFLECTION.md`, `tables.csv` — your marked deliverables.
- `evaluate.pbs` — Rome perf harness (run with `qsub evaluate.pbs` from your CX3 account).

## Build (laptop)

```bash
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18    # any Clang ≥ 16 will do
cmake --build build -j
OMP_NUM_THREADS=4 ./build/integrate
```

Recommended toolchain:
- macOS: `brew install llvm libomp cmake ninja`; use `/opt/homebrew/opt/llvm/bin/clang++`.
- Ubuntu / WSL: `apt install clang-18 libomp-18-dev cmake ninja-build`.

## What to do

1. Parallelise `integrate_parallel()` in `integrate.cpp` using `#pragma omp parallel for` + `reduction(+:sum)`.
2. **Try at least three schedules** (`static`, `dynamic, 64`, `guided`); measure each; pick the best.
3. Fill in `tables.csv` with your measured times, speedups, and efficiencies. The grader checks each row's `speedup = T(1)/T(P)` and `efficiency = speedup/P` are internally consistent (within 2 %); your numbers are *not* cross-checked against canonical timings.
4. Answer the 15 questions in `questions.md` via `answers.csv`.
5. Write `REFLECTION.md` per the template (sections fixed — CI checks format).

## Sanity-check under TSan locally before pushing

The CI gate runs Clang-18 + ThreadSanitizer + Archer; you can run the same check on your laptop:

```bash
clang++ -fopenmp -fsanitize=thread -g -O1 \
    integrate.cpp integrate_serial.cpp -o integrate_tsan
OMP_NUM_THREADS=4 ./integrate_tsan
```

If TSan reports `WARNING: ThreadSanitizer: data race` on a `reduction(+:sum)` build, that's a real bug — almost always a missing `default(none)` letting some accumulator default to `shared`. Silent (no warning) is the goal.

## Rubric (20 pts)

| Component | Pts | How measured |
|---|---|---|
| Build + TSan clean | 2 | Clang-18 + TSan + Archer on GH Actions |
| Correctness (graduated, per thread count) | 6 | `smart_diff.py` at `{1, 16, 64, 128}` on Rome (canonical re-run); 1.5 pts each |
| Reference-parallel-time perf | 5 | `min(1.0, T_ref(128) / T_student(128)) × 5` against the published `T_ref` measured on Rome; **correctness-gated** |
| `tables.csv` internal consistency | 1 | Per-row `speedup = T(1)/T(P)` and `efficiency = speedup/P` within 2 %. No canonical cross-check. |
| Style (clang-format / clang-tidy / cppcheck) | 2 | Lint workflow |
| MCQ (15 questions) | 2 | Deterministic auto-grading |
| REFLECTION.md format + completion | 1 | CI-format-check only (header presence + ≥ 50 words per required section). No numerical cross-check. |
| Reasoning question (instructor-marked, ≤ 100 words) | 1 | Manual 0/0.5/1 |

**Correctness gates performance.** If any thread count produces the wrong integral, the perf component is 0 regardless of timing.

## Performance target

A1 is graded on a **reference-parallel-time ratio**: your time at 128 threads vs the published reference time `T_ref(128)` measured by the instructor's reference parallel solution on Rome. The score formula `min(1.0, T_ref(128) / T_student(128)) × 5` caps at 5 (no extra credit for beating the reference by a wide margin). Aim to match or beat the reference.

The published `T_ref` value will be added to this README once the reference solution has been measured on Rome at the start of the cohort.

## Grading model

Grading is run once by the instructor at the **end of day 5** on a canonical CX3 Rome node. **All performance is correctness-gated** — any thread-count correctness fail zeros the perf component. No LLM is used in the summative grading path.

## What the formative CI checks (every push)

- **Build + correctness** at a small spread of thread counts (`{1, 2, 4, 8, 16}`) under Clang-18 + ThreadSanitizer + Archer OMPT.
- **Lint**: clang-format, clang-tidy, cppcheck.
- **REFLECTION format**: required headers present + each section ≥ 50 words. (Format only — no content scoring.)
- **Language check**: only English in Markdown and C++ comments.
- **No committed build artifacts**: `.o` / executables / `build/` are rejected.

There is **no Rome benchmark CI on student forks**. Performance is measured by the instructor at the end of the cohort on a CX3 Rome node. You can run `evaluate.pbs` on your own CX3 account to populate your `tables.csv`.

## Hygiene (10 pts, cohort-wide) + commit history

- A separate **10-pt Hygiene bucket** — build cleanliness, lint compliance (clang-format / clang-tidy / cppcheck), README / English readability — is graded once across A1+A2+A3 on your final state. Hygiene in *this* repo contributes. See `assessment/rubric.md` in the lectures repo for the breakdown.
- **Commit history** is *not* directly graded but is checked deterministically: see the lectures repo's `assessment/handouts/commit-history-guidance.md`. Patterns like a single mega-commit on day 5 with token messages are flagged for instructor review.

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

Brief released day 2 morning. A1 is completable by **end of day 2**. Day 5 final snapshot is graded.
