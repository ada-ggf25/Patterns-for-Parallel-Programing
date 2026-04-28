# ppp-openmp — Shared-memory programming with OpenMP

Days 2–4 of a 5-day parallel-programming course. Day 1 is covered by the sibling [`ppp-hpc-intro`](https://github.com/ese-ada-lovelace-2025/ppp-hpc-intro) course; day 5 is reserved for assessment iteration.

The course is pinned to [**OpenMP 5.1**](https://www.openmp.org/spec-html/5.1/openmp.html). The assessed kernels are:

- **A1** — numerical integration (parallel-for + reduction + schedule analysis)
- **A2** — Mandelbrot with a two-variant comparison (parallel-for vs tasks)
- **A3** — 3D Jacobi stencil with a core + extension split (NUMA / false-sharing / SIMD)

Assessment repo (students fork this): [`ppp-openmp-assessment`](https://github.com/ese-ada-lovelace-2025/ppp-openmp-assessment).

## Layout

```
slides/                 RevealJS decks: recap + day2 + day3 + day4
snippets/               Every slide code block is a real tested .cpp file here
  recap/  day2/         Plus per-day CMakeLists.txt and tests/test_*.cpp
  day3/   day4/
assessment/             Student-facing brief + rubric + templates + handouts
docs/                   Setup guides, toolchain matrix, LO blueprint, Rome inventory
```

The grader code lives in a separate **private** repo `ppp-openmp-grader`
(reference solutions, MCQ keys, `grade_cohort.sh`). It is run by the
instructor on a CX3 login node — there is no self-hosted GitHub Actions
runner.

## Quick start

### Build and test lecture snippets (laptop)

```bash
# macOS
brew install llvm libomp cmake ninja

# Ubuntu / WSL
sudo apt install clang-18 libomp-18-dev cmake ninja-build

# Both — vendor doctest, then run the three lanes:
curl -fsSL https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h \
  -o snippets/third_party/doctest.h
make snippets-test          # release lane (correctness, excludes RACE_DEMO + perf)
make snippets-tsan          # TSan race-demo lane (works on macOS without libarcher)
make snippets-perf          # perf lane (false-sharing soft delta)
```

For the **full** TSan suite (race-demo plus non-race correctness under TSan), libarcher is required — Linux + LLVM only:

```bash
ARCHER_LIB=/usr/lib/llvm-18/lib/libarcher.so make snippets-tsan-full
```

### Render slides

```bash
make html              # render landing page (slides/index.html) + four decks
make watch DECK=slides/day2   # live preview the active deck in a browser
```

`slides/index.html` is the week schedule (days 1–5, morning + afternoon split). It's the page to land on first; each day links into its own deck.

## Learning outcomes

See [`docs/outcomes.md`](docs/outcomes.md) for the full LO + Bloom + alignment blueprint. Every assessment component traces back to an outcome there.

## License

[Attribution-NonCommercial-ShareAlike 4.0 International](http://creativecommons.org/licenses/by-nc-sa/4.0/deed.en_US).
