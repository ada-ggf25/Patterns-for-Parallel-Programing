# Building the Examples

The repo's `examples/` directory is one CMake project that builds three targets — `pi_serial`, `pi_openmp`, `pi_mpi`. The top-level convenience target is wired through the repo's root `Makefile`.

## Top-level CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(ppp_intro_examples CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(serial)

find_package(OpenMP REQUIRED COMPONENTS CXX)
add_subdirectory(openmp)

find_package(MPI REQUIRED COMPONENTS CXX)
add_subdirectory(mpi)
```

Three subdirectories, three add_executable calls. OpenMP and MPI are discovered via `find_package` once each, providing imported targets the per-target CMakeLists files just link against.

## Per-target CMakeLists

```cmake
# serial/CMakeLists.txt
add_executable(pi_serial pi_serial.cpp)

# openmp/CMakeLists.txt
add_executable(pi_openmp pi_openmp.cpp)
target_link_libraries(pi_openmp PRIVATE OpenMP::OpenMP_CXX)

# mpi/CMakeLists.txt
add_executable(pi_mpi pi_mpi.cpp)
target_link_libraries(pi_mpi PRIVATE MPI::MPI_CXX)
```

Idiomatic modern CMake — link the imported target, no manual flags.

## On CX3

```bash
ssh me@login.cx3.hpc.ic.ac.uk
git clone <this repo> ic-hpc-intro
cd ic-hpc-intro/examples

ml tools/prod
ml GCC OpenMPI CMake

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

After this you have:

```
examples/build/
  serial/pi_serial
  openmp/pi_openmp
  mpi/pi_mpi
```

## Locally (Mac / Linux)

```bash
cd ic-hpc-intro/examples
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

./build/serial/pi_serial
OMP_NUM_THREADS=4 ./build/openmp/pi_openmp
mpiexec -n 4 ./build/mpi/pi_mpi
```

Requirements:

- CMake ≥ 3.16
- C++17 compiler with OpenMP (GCC or Clang+libomp)
- An MPI implementation (Open MPI, MPICH)

On macOS:

```bash
brew install gcc open-mpi
CXX=g++-14 cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

(Apple Clang ships without OpenMP, so use Homebrew GCC.)

## Top-level shortcut — `make examples-build`

The repo's root Makefile wraps the CMake flow:

```bash
# from the repo root
make examples-build       # equivalent to the cmake/cmake-build pair
make examples-clean       # rm -rf examples/build
```

## Why each `find_package` is required

`find_package(OpenMP REQUIRED COMPONENTS CXX)` and `find_package(MPI REQUIRED COMPONENTS CXX)` both set `REQUIRED` — meaning configuration **fails immediately** if either is missing. This is intentional:

- A silent fall-back to "no OpenMP" would build the binary, run it, produce a correct answer at single-thread speed, and you'd never know.
- A silent fall-back to "no MPI" would just fail at link time with cryptic symbol errors.

Better to fail at configure time with a clear "OpenMP not found" message.

## Related

- [[../openmp/Building OpenMP]]
- [[../mpi/Building MPI]]
- [[Running on CX3]]
- [[Running Locally]]
