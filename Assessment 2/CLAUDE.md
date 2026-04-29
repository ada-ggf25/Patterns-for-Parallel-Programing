# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Assignment overview

PPP-OpenMP Assessment 2: estimate the Mandelbrot set area on a 5000×5000 grid using two OpenMP parallelism strategies, then measure and justify a recommendation. Target: 30 marks.

## Build

```bash
# Configure (Clang 18 preferred; any Clang ≥ 16 works)
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18

# Build both variants
cmake --build build -j

# Run
OMP_NUM_THREADS=4 ./build/mandelbrot_for
OMP_NUM_THREADS=4 ./build/mandelbrot_tasks
```

Ubuntu/WSL dependencies: `apt install clang-18 libomp-18-dev cmake ninja-build`

## ThreadSanitizer check (run before pushing)

```bash
clang++ -fopenmp -fsanitize=thread -g -O1 \
    mandelbrot_for.cpp mandelbrot_serial.cpp -o mandelbrot_for_tsan
OMP_NUM_THREADS=4 ./mandelbrot_for_tsan

clang++ -fopenmp -fsanitize=thread -g -O1 \
    mandelbrot_tasks.cpp mandelbrot_serial.cpp -o mandelbrot_tasks_tsan
OMP_NUM_THREADS=4 ./mandelbrot_tasks_tsan
```

## CX3 Rome benchmark (performance data for tables.csv / CHOICE.md)

```bash
qsub evaluate.pbs   # from your CX3 account
```

Outputs `perf-results-a2.json` (thread counts 1/16/64/128, both variants). The grader cross-checks `CHOICE.md`'s `measured_tasks_s` / `measured_for_s` against this file (within 5%).

## Architecture

- `mandelbrot.h` — shared constants (`NPOINTS=5000`, `MAXITER=1000`, `TILE=100`) and function declarations. Both variants must return the same `long` count as `mandelbrot_serial`.
- `mandelbrot_serial.cpp` — reference implementation. **Do not edit.** Uses real-axis symmetry: only iterates the upper half (`j ∈ [0, NPOINTS/2)`) and doubles the count, halving the work.
- `mandelbrot_for.cpp` — student implementation using `#pragma omp parallel for`. Has a serial fallback stub; replace the nested loops with a parallelised reduction. Consider `schedule(dynamic)` or `schedule(dynamic, chunk)` to handle the ~100× per-pixel cost variation near the boundary.
- `mandelbrot_tasks.cpp` — student implementation using `#pragma omp task` or `#pragma omp taskloop`. Pre-structured with `count_tile_upper()` helper that processes one TILE×TILE block. Spawn one task per tile origin `(i0, j0)` inside a `parallel + single` region; accumulate with `reduction` or `atomic`.

Both variants must mirror the serial symmetry convention: multiply upper-half escapes by 2, handle the centre row exactly once when `NPOINTS` is odd.

## Deliverables and their format constraints

| File | What to fill in | Graded by |
|---|---|---|
| `mandelbrot_for.cpp` | Parallel implementation | TSan + correctness CI |
| `mandelbrot_tasks.cpp` | Task-parallel implementation | TSan + correctness CI |
| `tables.csv` | Times, speedups, efficiencies at 1/16/64/128 threads | Internal consistency: `speedup = T(1)/T(P)`, `efficiency = speedup/P` within 2% |
| `CHOICE.md` | YAML header: `recommended`, `measured_tasks_s`, `measured_for_s`, optional `justification_keyword` | Deterministic parse |
| `answers.csv` | One letter (A/B/C/D) per q01–q15 | Auto-graded |
| `REFLECTION.md` | Sections 1–4 + Reasoning question, each ≥ 50 words | Format-only CI; content graded by human |

`CHOICE.md` header format:
```yaml
---
recommended: tasks        # tasks | parallel_for
measured_tasks_s: 4.21
measured_for_s: 5.03
justification_keyword: irregular_load_balance   # omit if recommending your faster variant
---
```

Valid `justification_keyword` values (only required when recommending the slower variant): `irregular_load_balance`, `scales_better_at_128T`, `simpler_to_maintain`, `future_proof_for_dynamic_work`.

## CI checks (every push)

1. **lint** — clang-format-20, clang-tidy-20, cppcheck against `.clang-format` / `.clang-tidy`
2. **correctness** — Clang-18 + TSan + Archer OMPT; both variants at thread counts {1,2,4,8,16}; output compared against `expected_output.txt` via `bin/smart_diff.py`
3. **reflection-format** — all required `## Section N` headers present, each ≥ 50 words
4. **language-check** — English only in Markdown and C++ comments
5. **generated-files-check** — rejects committed `.o`, executables, `build/` directory

## Hard constraints

- Do not rename source files or public function signatures (`mandelbrot_parallel_for`, `mandelbrot_tasks`, `mandelbrot_serial`).
- Do not add new headers, dependencies, or third-party libraries.
- Do not modify `.github/workflows/`, `bin/smart_diff.py`, or other harness files.
- Do not commit build artifacts (binaries, `.o` files, `build/` directory).
