# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

Imperial College PPP-OpenMP Assessment 1: parallelise a composite-trapezoid numerical integration of a deliberately non-uniform `f(x)` over `[0,1]` using OpenMP. Target: 20 marks.

## Build

```bash
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18
cmake --build build -j
OMP_NUM_THREADS=4 ./build/integrate
```

On Ubuntu/WSL: `apt install clang-18 libomp-18-dev cmake ninja-build`.

## Sanity-check (TSan)

```bash
clang++ -fopenmp -fsanitize=thread -g -O1 \
    integrate.cpp integrate_serial.cpp -o integrate_tsan
OMP_NUM_THREADS=4 ./integrate_tsan
```

No `WARNING: ThreadSanitizer: data race` in output = clean.

## Correctness check

```bash
OMP_NUM_THREADS=4 ./build/integrate > out.txt
python3 bin/smart_diff.py out.txt expected_output.txt
```

`smart_diff.py` does float-tolerant token comparison (rtol=1e-6, atol=1e-8).

## Performance benchmark (CX3 Rome only)

```bash
qsub evaluate.pbs   # from a CX3 login node
```

Produces `perf-results-a1.json`. Thread ladder: `{1, 16, 64, 128}`.

## Linting

CI uses clang-format-20, clang-tidy-20, and cppcheck. To check locally:

```bash
clang-format-18 --dry-run --Werror integrate.cpp integrate.h
clang-tidy-18 -p build integrate.cpp integrate_serial.cpp
cppcheck --enable=performance,warning,style --quiet --project=build/compile_commands.json
```

Style: LLVM-based, 4-space indent, 100-column limit, braces on their own line after functions/classes (never after control statements). See `.clang-format`.

## Architecture

- `integrate_serial.cpp` — **do not edit**. Contains `f(x)` with a "spike" at `x ∈ (0.3, 0.4)` (~10× heavier per call) and the reference serial `integrate_serial()`.
- `integrate.h` — shared declarations for `f`, `integrate_serial`, and `integrate_parallel`.
- `integrate.cpp` — **your implementation**. Contains `integrate_parallel()` (currently a serial fallback) and `main()` (do not modify `main()`).

The key design constraint: `f(x)` has non-uniform per-call cost. Static scheduling leaves one thread doing the entire spike region; dynamic or guided scheduling distributes the heavy chunk across threads. The CI correctness gate runs `{1, 2, 4, 8, 16}` threads under Clang-18 + TSan + Archer OMPT.

## Deliverables

| File | What to fill |
|---|---|
| `integrate.cpp` | Parallelise `integrate_parallel()` with `#pragma omp parallel for` + `reduction(+:sum)` |
| `answers.csv` | One letter (A/B/C/D) per q01–q15 |
| `tables.csv` | `measured_time_s`, `measured_speedup`, `measured_efficiency` for threads `{1, 16, 64, 128}` |
| `REFLECTION.md` | Prose for Section 1–4 (≥50 words each) + Reasoning question (≤100 words) |

## CI checks (every push)

- **correctness**: build + TSan + Archer at thread counts `{1,2,4,8,16}`; `smart_diff.py` against `expected_output.txt`
- **lint**: clang-format, clang-tidy, cppcheck
- **reflection-format**: required `## Section N` headers present + ≥50 words each
- **language-check**: English only in `.md` files and C++ comments
- **generated-files-check**: rejects committed `.o`, executables, `build/` directory

## Constraints

- Do not rename source files or public function signatures.
- Do not modify `integrate_serial.cpp`, `main()` in `integrate.cpp`, `.github/workflows/`, or files under `bin/`.
- Do not add headers, new dependencies, or third-party libraries.
- Do not commit the `build/` directory or any compiled artifacts.
- `default(none)` on the parallel region is required — missing it can cause silent data races on the accumulator that TSan will catch.
