# Parallel Patterns in Programming — intro to HPC on CX3

Teaching materials for the introductory HPC session at Imperial College London,
covering Imperial's **CX3** cluster, environment **modules**, **PBS** job
submission, and a hands-on tour of **OpenMP** and **MPI** on a single running
example ($\pi$ by numerical integration).

The repository contains:

- `cx3-hpc-intro.qmd` — single-source Quarto deck rendered to both Beamer PDF
  and RevealJS HTML.
- `examples/` — three implementations (serial, OpenMP, MPI) of the same
  $\pi$-by-midpoint-rule program, plus matching PBS submission scripts. See
  [`examples/README.md`](examples/README.md) for build/run details on CX3 and
  locally.
- `check_queue_busyness.sh` — diagnostic for inspecting CX3 queue load (must
  run on a PBS login node).

## Course agenda

This deck is the kickoff for a three-week parallel-programming course.

### This week — HPC intro & OpenMP (Mon 27 Apr – Fri 1 May)

- **Today:** Introduction to HPC on CX3 — modules, PBS, plus illustrative
  OpenMP and MPI examples (this deck).
- Rest of the week: OpenMP in depth — parallel regions, data sharing, work
  sharing, reductions.
- **OpenMP assessment due 16:00 Friday.**

### Week 2 — MPI (Mon 4 May – Fri 8 May)

- Distributed-memory parallelism with MPI.
- Lecturer: **Prof Stephen Neethling**.

### Week 3 — MPI continued (Mon 11 May – Fri 15 May)

- Further MPI topics and assessment.
- Lecturer: **Prof Stephen Neethling**.

## Building the slides locally

The same `cx3-hpc-intro.qmd` produces both the HTML deck and the Beamer PDF;
you pick which via the `make` target.

### Dependencies

| Target | Requires |
|--------|----------|
| `make html` | **Quarto ≥ 1.4** (Pandoc, Deno, and Dart Sass are bundled). |
| `make pdf`  | Quarto, plus a LaTeX distribution that provides **LuaLaTeX**, **Beamer**, and the standard Beamer themes (Madrid / seahorse / professionalfonts). |

#### macOS

```bash
brew install --cask quarto       # Quarto CLI
brew install --cask mactex       # full TeX Live; everything needed for the PDF
```

If you want a smaller install, `brew install --cask basictex` plus
`sudo tlmgr install beamer pgf etoolbox translator` is usually enough; reopen
the shell so `lualatex` lands on `PATH`.

#### Linux (Debian / Ubuntu)

Install Quarto from the official `.deb` at
<https://quarto.org/docs/get-started/> (the `quarto` package in distro repos is
typically out of date), then:

```bash
sudo apt install texlive-luatex texlive-latex-recommended \
                 texlive-latex-extra texlive-fonts-recommended
```

#### Linux (Fedora / RHEL)

```bash
sudo dnf install texlive-scheme-medium texlive-beamer
```

Quarto: download the `.rpm` from <https://quarto.org/docs/get-started/>.

### Verify your toolchain

```bash
quarto check        # confirms Pandoc, LaTeX, Chromium engines
lualatex --version  # should print "This is LuaTeX..."
```

### Build commands

```bash
make html     # cx3-hpc-intro.html via RevealJS
make pdf      # cx3-hpc-intro.pdf via Beamer (LuaLaTeX)
make all      # both
make watch    # live-preview the HTML deck in a browser
make clean    # remove rendered outputs and LaTeX intermediates
```

The HTML deck is self-contained except for an assets folder
(`cx3-hpc-intro_files/`) created alongside it; serve or share both together.

## Building the examples locally

Brief version (full notes in [`examples/README.md`](examples/README.md)):

```bash
make examples-build       # configure + build serial / openmp / mpi
./examples/build/serial/pi_serial
OMP_NUM_THREADS=4 ./examples/build/openmp/pi_openmp
mpiexec -n 4 ./examples/build/mpi/pi_mpi
```

Requires CMake ≥ 3.18, a C++ compiler with OpenMP, and an MPI implementation.
On macOS use Homebrew's GCC and Open MPI (`brew install gcc open-mpi`) — Apple
Clang ships without OpenMP support by default. On Linux any recent GCC plus
`libopenmpi-dev` (or MPICH) works.

## Building on CX3

For the examples, load the module stack first:

```bash
ml tools/prod
ml GCC OpenMPI CMake
make examples-build
```

PBS submission scripts live in `examples/{serial,openmp,mpi}/`. Submit from
`examples/`, not from the per-target subdirectory — the scripts reference the
build output relative to the submit CWD.
