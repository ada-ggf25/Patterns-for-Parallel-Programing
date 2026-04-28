# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

Teaching materials for an introductory HPC session at Imperial College London. The single source of truth for the slides is `cx3-hpc-intro.qmd` (Quarto Markdown), rendered to both a RevealJS HTML deck and a Beamer PDF. The `examples/` directory contains three C++ implementations (serial, OpenMP, MPI) of the same π-by-midpoint-rule program, plus PBS job submission scripts for CX3.

## Build commands

### Slides

```bash
make html          # render RevealJS HTML (cx3-hpc-intro.html + cx3-hpc-intro_files/)
make pdf           # render Beamer PDF via LuaLaTeX
make all           # both
make watch         # live-preview HTML in browser
make clean         # remove all rendered outputs and LaTeX intermediates
```

`make html` only needs Quarto ≥ 1.4. `make pdf` also needs a LaTeX distribution with LuaLaTeX and Beamer (e.g. `texlive-luatex texlive-latex-extra` on Debian/Ubuntu).

### C++ examples

```bash
make examples-build   # cmake configure + build (Release) into examples/build/
make examples-clean   # rm -rf examples/build
```

Run locally after building:

```bash
./examples/build/serial/pi_serial
OMP_NUM_THREADS=4 ./examples/build/openmp/pi_openmp
mpiexec -n 4 ./examples/build/mpi/pi_mpi
```

Requires CMake ≥ 3.16, a C++ compiler with OpenMP, and an MPI implementation. On macOS, use `brew install gcc open-mpi` (Apple Clang has no OpenMP by default).

### On CX3 (PBS cluster)

```bash
ml tools/prod
ml GCC OpenMPI CMake
make examples-build
# Submit from examples/ (not the per-target subdirectory)
qsub examples/serial/pi_serial.pbs
qsub examples/openmp/pi_openmp.pbs
qsub examples/mpi/pi_mpi.pbs
```

## Architecture

```
cx3-hpc-intro.qmd      # single-source Quarto deck (Beamer + RevealJS)
_quarto.yml            # project-level Quarto config (freeze: auto)
Makefile               # build targets for slides and examples
examples/
  CMakeLists.txt       # top-level CMake; requires OpenMP + MPI
  serial/              # plain C++ baseline
  openmp/              # #pragma omp parallel for reduction
  mpi/                 # MPI_Reduce across ranks
  benchmarks/
    numa_latency/      # NUMA latency measurement (C + PBS script)
images/                # image assets embedded in the deck
.github/workflows/     # publish-html.yml: renders HTML and deploys to gh-pages on push to main
```

The three example programs all compute the same integral with `N = 1e9` iterations hard-coded. Edit the `const long long n = …` line in each source to change the problem size for demos or benchmarking.

## CI / publishing

`publish-html.yml` runs on every push to `main`: it renders `cx3-hpc-intro.qmd` to RevealJS and force-deploys the result to the `gh-pages` branch (orphan, so history is not kept). The HTML deck is served from there as `index.html`.

## CX3-specific notes

- PBS scripts default to `cpu_type=rome` (AMD EPYC 7742, 128 cores, 8 NUMA domains). Change to `cpu_type=icelake` for a simpler 2-NUMA topology.
- Load `OpenMPI` (not just `GCC`) to get `hwloc` (`lstopo`) and `numactl` on PATH.
- PBS jobs should be submitted from `examples/`, not from subdirectories, because the PBS scripts reference build outputs relative to the submit CWD.
- After loading new modules in an existing shell, run `hash -r` to clear bash's command cache.
