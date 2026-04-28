# Toolchain matrix

What-works-where, in one place. Use this to decide which tool to reach for and on which machine.

| Tool | macOS laptop | Linux / WSL laptop | GitHub Actions (Ubuntu) | CX3 Rome |
|---|---|---|---|---|
| Clang with `-fopenmp-version=51` | ✅ Homebrew `llvm` (clang 22+) | ✅ `clang-18` from apt | ✅ `clang-18` in workflow | ⚠️ check `module avail Clang` — not all CX3 Rome nodes ship it; fall back to GCC |
| GCC with `-fopenmp` | ⚠️ via Homebrew, rarely needed | ✅ `gcc` default | ✅ | ✅ primary on-node compiler (`module load tools/prod GCC`) |
| `clang-format` | ✅ | ✅ | ✅ | ⚠️ slower on login nodes; prefer laptop |
| `clang-tidy` | ✅ | ✅ | ✅ | ⚠️ same |
| `cppcheck` | ✅ (brew) | ✅ (apt) | ✅ | ✅ (module) |
| `quarto` | ✅ | ✅ | ✅ | ❌ not needed on CX3 |
| ThreadSanitizer runtime | ✅ Clang (race-demo lane only — Homebrew LLVM ships no `libarcher`) | ✅ Clang | ✅ Clang + Archer (full TSan suite) | ⚠️ TSan is enforced on GH Actions, not CX3 |
| Archer OMPT tool | ❌ not in Homebrew LLVM | ✅ via LLVM `libomp-18-dev` (look for `libarcher.so`) | ✅ | ⚠️ depends on Clang module availability |
| `hyperfine` | ✅ (brew) | ✅ (apt) | n/a | ✅ (cargo install via `module load Rust`) |
| `numactl` | ❌ (not applicable — no NUMA on laptops) | ⚠️ on NUMA servers only | n/a | ✅ (module) |
| `STREAM` benchmark | ⚠️ measures laptop, not the grading target | ⚠️ same | n/a | ✅ **the canonical STREAM measurement lives on CX3 Rome** |
| `likwid-perfctr` | ❌ | ⚠️ if installed + PMU access | n/a | ❓ check `module avail likwid`; relegated to feedback only, not grading |

## Rule of thumb

- **Correctness** — enforced on GitHub Actions (Ubuntu + Clang + TSan + Archer for the full suite; release-only on macOS). Must pass before you submit to CX3.
- **Perf (reference-parallel-time / roofline)** — measured by the instructor on CX3 Rome at end of cohort via `grade_cohort.sh` in the private grader repo. There is no self-hosted runner.
- **Style / lint** — enforced everywhere; do it locally (pre-commit) to avoid push-wait-fail cycles.

## Canonical thread-count ladder (Rome)

`{1, 16, 64, 128}` — serial → one NUMA domain (16 cores per domain on NPS=4) → one socket → full dual-socket node.

See `docs/rome-inventory.md` for the measured STREAM bandwidth, peak FLOPs, and module names available.
