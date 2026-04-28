# Local setup — macOS

For A1/A2/A3 you develop on your laptop, push to GitHub (where CI runs lint + TSan + correctness), and submit to CX3 Rome only for the performance benchmark. This page is the laptop half on macOS (Apple Silicon and Intel both work).

## Install the toolchain

```bash
# Homebrew — skip if already installed:
# /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

brew install llvm libomp cmake ninja pre-commit hyperfine
brew install --cask rstudio  # optional, only if you want a Quarto UI
brew install quarto          # for rendering the slide decks
```

Homebrew Clang 22.x defaults to OpenMP 5.1 (`_OPENMP=202011`) with no extra flags.

## Point CMake at Homebrew Clang, not Apple Clang

Apple's `/usr/bin/clang` is **not** the same as Homebrew Clang. OpenMP works on both but the OpenMP 5.1 features we use are only complete on Homebrew Clang. Always:

```bash
export CC=/opt/homebrew/opt/llvm/bin/clang
export CXX=/opt/homebrew/opt/llvm/bin/clang++
```

(On Intel Macs the path is `/usr/local/opt/llvm/bin/...`.)

## Fetch doctest (required once after cloning)

The doctest single-header is vendored *at build time* rather than committed:

```bash
curl -fsSL https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h \
  -o snippets/third_party/doctest.h
```

## Build and test the lecture snippets

```bash
make snippets-test
```

Expected output: every `day2_*` / `day3_*` / `day4_*` test passes.

## TSan (correctness under races)

```bash
make snippets-tsan
```

On macOS, `OMP_PLACES=cores OMP_PROC_BIND=close` + Homebrew's `libomp` works cleanly with TSan. Race-demo snippets are labelled and their tests expect TSan to fire.

## Working on an A1/A2/A3 submission

Clone your forked `ppp-openmp-assessment` repo (not this lectures repo — they are separate). In that repo:

```bash
mkdir -p build && cd build
cmake ..
make                     # builds the per-assignment targets
./integrate              # run A1
OMP_NUM_THREADS=4 ./integrate
```

For reference outputs: `diff` against `assignment-N/expected_output.txt` via `bin/smart_diff.py` (float-tolerant — spaces and trailing newlines don't matter).

## Pre-commit hooks (recommended)

```bash
pre-commit install
```

This runs clang-format + markdownlint + a few other lightweight checks on every commit. CI enforces the same checks, so local pre-commit saves you a push-wait-fail cycle.

## Troubleshooting

- **"OpenMP not found" during cmake**: you're on Apple Clang. Re-export `CC` and `CXX` to Homebrew paths and delete your `build/` dir.
- **Link error `___kmpc_for_static_init_4`**: similarly, the linker is picking up Apple Clang. Force Homebrew clang in cmake.
- **TSan spurious reports under Archer**: ensure `OMP_TOOL=archer` is exported and `libarcher.dylib` resolves (`brew --prefix libomp`/lib).
