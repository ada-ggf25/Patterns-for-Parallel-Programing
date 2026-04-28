# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

Teaching materials for an OpenMP shared-memory programming course (days 2–4 of a 5-day parallel-programming module at Imperial College London). The three assessed kernels are:

- **A1** — numerical integration (parallel-for + reduction + schedule analysis)
- **A2** — Mandelbrot with a two-variant comparison (parallel-for vs tasks)
- **A3** — 3D Jacobi stencil with NUMA / false-sharing / SIMD extension; student picks ONE extension branch

Assessment uses `{1, 16, 64, 128}` threads as the canonical thread ladder on CX3 Rome (128-core dual EPYC 7742). See `assessment/rubric.md` for the full 100-pt breakdown and `docs/toolchain-matrix.md` for compiler/tool availability per environment.

## Build commands

### Slides

```bash
make html              # render landing page + four RevealJS decks (runs partials first)
make watch DECK=slides/day2   # live-preview the active deck (default: day2)
make clean             # remove rendered outputs, partials, and freeze cache
```

### Snippets (C++ lecture examples)

```bash
# Install doctest header first (one-time):
curl -fsSL https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h \
  -o snippets/third_party/doctest.h

make snippets-test          # build + run correctness tests (excludes RACE_DEMO + perf)
make snippets-tsan          # build + run race-demo lane under TSan (macOS-compatible)
make snippets-perf          # run perf-labelled assertions (false-sharing delta etc.)
make snippets-clean         # rm -rf snippets/build

# Full TSan suite (Linux + LLVM with libarcher — suppresses OpenMP false positives):
ARCHER_LIB=/usr/lib/llvm-18/lib/libarcher.so make snippets-tsan-full
```

Run a single ctest by name (after `snippets-build`):

```bash
ctest --test-dir snippets/build/release -R day2_reduction_sum --output-on-failure
```

### Dependencies

- Quarto ≥ 1.4 (slides)
- CMake ≥ 3.20, C++20 compiler with OpenMP (snippets)
- macOS: `brew install llvm libomp cmake ninja`
- Ubuntu/WSL: `sudo apt install clang-18 libomp-18-dev cmake ninja-build`

### Lint

```bash
make lint              # clang-format dry-run + markdownlint + lychee link check
pip install pre-commit && pre-commit install   # gate same checks on every commit
```

## Architecture

```
slides/                  RevealJS decks: index + day2 + day3 + day4 (.qmd source)
slides/tools/            build_partials.py (converts snippets → .qmd partials)
                         render_*.py (generate committed PNGs — run by hand, not CI)
slides/_partials/        Auto-generated .qmd partials — do NOT edit by hand
snippets/                Every slide code block is a real, tested .cpp file
  recap/                 Roofline calc demo (used in the day2 recap slide)
  day2/ day3/ day4/      Source files + tests/test_*.cpp per day
  cmake/                 EnableTSan.cmake, EnableUBSan.cmake, OpenMPStrict.cmake
  third_party/           doctest.h (vendored, not committed — fetch manually)
assessment/
  rubric.md              Canonical 100-pt rubric (A1=20, A2=30, A3-core=25, A3-ext=15, hygiene=10)
  templates/             Per-assessment student-facing templates (REFLECTION, CHOICE, EXTENSION)
  handouts/              commit-history-guidance.md
docs/
  local-setup-linux.md   WSL / Ubuntu setup guide
  local-setup-macos.md   macOS setup guide
  toolchain-matrix.md    What-works-where (compiler, TSan, numactl, STREAM) per environment
  cx3-benchmarking.md    How to run and record performance on CX3
  outcomes.md            Learning-outcome blueprint
  rome-inventory.md      Measured STREAM bandwidth, peak FLOPs, module names on Rome
requirements.txt         Python deps for slides/tools (build_partials, render_*.py)
```

### Slides ↔ snippets pipeline

`slides/tools/build_partials.py` reads every `snippets/<day>/<name>.cpp` and emits:
- `slides/_partials/<day>/<name>.qmd` — the whole file
- `slides/_partials/<day>/<name>__<region>.qmd` — per `// snippet-begin: <region>` / `// snippet-end: <region>` marker pair

Slides include code via `{{< include slides/_partials/... >}}`. This keeps slides and tests in sync — the same `.cpp` is both compiled and displayed. Never edit `_partials/` directly; edit the source `.cpp` and re-run `make partials` (or `make html`).

### Test labelling

CTest labels control which test lane runs:

| Label | Meaning | Excluded from |
|---|---|---|
| `RACE_DEMO` | Intentional data race; passes only under TSan via `PASS_REGULAR_EXPRESSION` | `snippets-test` |
| `perf` | Behavioural-performance assertions (noisy on VMs) | `snippets-test` |

### clang-tidy profile

`.clang-tidy` enables `modernize-*`, `readability-*`, `performance-*`, `bugprone-*`, `openmp-*`, `cppcoreguidelines-*` with didactic exclusions (magic numbers, C-style arrays, raw pointer arithmetic allowed in teaching code).

## CI

| Workflow | Trigger | What it does |
|---|---|---|
| `snippets-build.yml` | push / PR | Ubuntu: build + `snippets-test` + `snippets-tsan-full` (with libarcher) |
| `snippets-macos.yml` | push / PR | macOS: build + `snippets-test` + `snippets-tsan` (race-demo lane only) |
| `render-slides.yml` | push to main | Renders HTML and deploys to `gh-pages` |
| `lint-cpp.yml` | push / PR | clang-format check |
| `lint-quarto.yml` | push / PR | markdownlint + lychee link check |
