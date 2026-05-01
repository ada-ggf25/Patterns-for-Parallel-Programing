# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Imperial PPP OpenMP Assessment 3 — a 7-point 3D Jacobi stencil (`NX=NY=NZ=512`, 100 steps, working set ≈ 2.1 GB). The task is to parallelise `jacobi_step()` in `core/stencil.cpp` and implement exactly one extension (`numa_first_touch`, `false_sharing`, or `simd`).

## Build

```bash
cmake -B build -S . -DCMAKE_CXX_COMPILER=clang++-18
cmake --build build -j
OMP_NUM_THREADS=4 ./build/stencil
```

On Ubuntu/WSL: `apt install clang-18 libomp-18-dev cmake ninja-build`.

Run correctness check manually:
```bash
OMP_NUM_THREADS=16 ./build/stencil | python3 bin/smart_diff.py - expected_output.txt
```

## Lint

```bash
clang-format-18 --dry-run --Werror $(git ls-files '*.cpp' '*.h')
clang-tidy-18 -p build $(git ls-files '*.cpp')
cppcheck --enable=performance,warning,style --error-exitcode=1 --project=build/compile_commands.json
```

Style: LLVM-based, 4-space indent, 100-column limit, braces on new line after functions/classes/structs (see `.clang-format`).

## CI checks (every push)

1. **lint** — clang-format, clang-tidy, cppcheck (uses LLVM 20 in CI).
2. **correctness** — builds with Clang 18 + ThreadSanitizer + Archer OMPT; runs core at `{1,2,4,8,16}` threads and chosen extension at `{1,4,8}` threads; output compared via `bin/smart_diff.py` against `expected_output.txt`.
3. **reflection-format** — `REFLECTION.md` must have `## Section 1–5` and `## Reasoning question` headers, each Section ≥ 50 words.
4. **language-check** — English only in `.md` files and C++ comments.
5. **generated-files-check** — no `.o`, binaries, or `build/` directory committed.

## Architecture

```
core/stencil.h       — grid constants (NX/NY/NZ/NSTEPS), jacobi_step() and checksum() declarations
core/stencil.cpp     — student implementation: jacobi_step(), checksum(), init(), main()
extension/<name>/    — one .cpp per variant; CMake auto-discovers any .cpp placed here
```

The grid uses row-major indexing: `idx(i,j,k) = i*NY*NZ + j*NZ + k`. `jacobi_step` reads from `u`, writes to `u_next`; `main()` double-buffers with `std::swap` each step.

Extension targets are named `ext_<extension>_<filename>` and must produce the same checksum as the core.

## Key constraints

- Do **not** rename source files, public function signatures, or modify `.github/workflows/`.
- Do **not** touch `bin/smart_diff.py` or other harness files.
- Do **not** add new headers, dependencies, or third-party libraries.
- Performance is graded once on a Rome node (128-core, 8-NUMA-domain EPYC 7742) — correctness must pass at `{1,16,64,128}` threads or the performance component is zeroed.

## Deliverables checklist

- `core/stencil.cpp` — `jacobi_step()` parallelised with `#pragma omp parallel for`.
- `extension/<chosen>/` — implementation files for the chosen extension.
- `EXTENSION.md` — structured header with `chosen`, `before_time_s`, `after_time_s`, `delta_percent` (internally consistent within ±10%).
- `answers.csv` — one letter per question (q01–q15).
- `tables.csv` — speedup/efficiency table; `speedup = T(1)/T(P)`, `efficiency = speedup/P`, within 2%.
- `REFLECTION.md` — Sections 1–5 + Reasoning question, each ≥ 50 words.
