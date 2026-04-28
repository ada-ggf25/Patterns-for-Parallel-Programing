# Building an MPI Program

MPI source files compile and link against the MPI runtime through a wrapper compiler.

## Plain command line — `mpicxx`

```bash
ml tools/prod GCC OpenMPI
mpicxx -O3 pi_mpi.cpp -o pi_mpi
```

`mpicxx` is a *wrapper*: under the hood it invokes `g++` with the right `-I`, `-L`, and `-l` flags for the MPI implementation you loaded. To see what it does:

```bash
mpicxx --showme               # OpenMPI: prints the underlying command
mpicxx --showme:compile       # just the include flags
mpicxx --showme:link          # just the link flags
```

There are sister wrappers:

| Wrapper | Language |
|---|---|
| `mpicc`  | C |
| `mpicxx` (or `mpic++`) | C++ |
| `mpif90` / `mpifort` | Fortran |

## CMake — what the repo uses

```cmake
find_package(MPI REQUIRED COMPONENTS CXX)
add_executable(pi_mpi pi_mpi.cpp)
target_link_libraries(pi_mpi PRIVATE MPI::MPI_CXX)
```

`find_package(MPI REQUIRED)`:

- Detects the MPI implementation (Open MPI, MPICH, Intel MPI, ...).
- Sets up `MPI::MPI_CXX` as an imported target carrying include paths, library paths, and link flags.
- Fails configuration if MPI isn't available.

You **don't call `mpicxx` yourself** when using CMake — CMake invokes the underlying `g++` directly with the discovered flags.

## On the course examples

The whole CMake config for MPI is two lines (`examples/mpi/CMakeLists.txt`):

```cmake
add_executable(pi_mpi pi_mpi.cpp)
target_link_libraries(pi_mpi PRIVATE MPI::MPI_CXX)
```

The top-level `examples/CMakeLists.txt` does `find_package(MPI REQUIRED COMPONENTS CXX)` once.

## Module dependencies

To build *and* run MPI on CX3, you need:

```bash
ml tools/prod
ml GCC OpenMPI
```

`OpenMPI` is the runtime. Without it loaded:

- The build will fail (CMake won't find MPI).
- A pre-built binary will fail to run with linker errors at startup.

See [[../modules/Loading Combos]].

## On macOS

```bash
brew install gcc open-mpi
CXX=g++-14 cmake -B build ...
```

System Apple Clang doesn't ship MPI; Homebrew's Open MPI does. Use the Homebrew `g++` so OpenMP also works.

## Related

- [[mpiexec]] — running the built binary.
- [[MPI PBS Script]] — submitting on CX3.
- [[../examples/Building the Examples]] — full project build.
- [[../modules/Loading Combos]]
