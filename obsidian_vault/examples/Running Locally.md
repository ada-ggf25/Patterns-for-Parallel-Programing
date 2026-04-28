# Running the Examples Locally

You can build and run all three examples on a Mac or Linux laptop without any cluster access. Useful for testing changes before you submit.

## Build

```bash
cd ic-hpc-intro/examples
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Requirements:

- CMake ≥ 3.16
- C++17 compiler with OpenMP support
- An MPI implementation (Open MPI, MPICH, etc.)

See [[Building the Examples]] for full details.

## Run

```bash
./build/serial/pi_serial
OMP_NUM_THREADS=4 ./build/openmp/pi_openmp
mpiexec -n 4 ./build/mpi/pi_mpi
```

Differences from running on CX3:

- **No `qsub`.** Just run the binaries directly.
- **Explicit `-n` for `mpiexec`.** Off the cluster, you must specify rank count.
- **Explicit `OMP_NUM_THREADS`.** No PBS to set it for you.

## On macOS

Apple Clang ships without OpenMP support and macOS has no system MPI. Install both via Homebrew:

```bash
brew install gcc open-mpi
```

Use Homebrew's GCC (not Apple Clang) for the build:

```bash
CXX=g++-14 cmake -B build -DCMAKE_BUILD_TYPE=Release   # adjust version
cmake --build build -j
```

If `find_package(OpenMP)` still fails after this, double-check that CMake is picking `g++-14` and not Apple Clang — `cmake --build build -v` shows the actual compile command.

## On Linux

Distro packages cover everything. On Debian / Ubuntu:

```bash
sudo apt install build-essential cmake libopenmpi-dev openmpi-bin
```

On Fedora:

```bash
sudo dnf install gcc-c++ cmake openmpi openmpi-devel
module load mpi/openmpi-x86_64       # Fedora's MPI module-load idiom
```

## Sanity checks

After building, verify each binary runs and prints something sensible:

```bash
$ ./build/serial/pi_serial
n=1000000000  pi=3.141592653589821  time=10.234s

$ OMP_NUM_THREADS=4 ./build/openmp/pi_openmp
n=1000000000  threads=4  pi=3.141592653589811  time=2.671s

$ mpiexec -n 4 ./build/mpi/pi_mpi
n=1000000000  ranks=4  pi=3.141592653589811  time=2.598s
```

Times are wildly hardware-dependent — a recent laptop will be much faster than a Rome compute node for a single thread, and much slower at 16 threads.

## When to use this vs CX3

| Task | Where |
|---|---|
| Edit code, see if it compiles | local |
| Quick correctness check (`pi ≈ 3.14159`) | local |
| Benchmark for assignment / report | CX3 (consistent hardware) |
| Investigate scaling beyond your laptop's core count | CX3 |
| Practice the build, debug syntax errors | local |

## Related

- [[Building the Examples]]
- [[Running on CX3]]
- [[pi_serial]] / [[pi_openmp]] / [[pi_mpi]]
