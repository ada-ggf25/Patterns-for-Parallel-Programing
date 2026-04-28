# Building an OpenMP Program

OpenMP support in modern GCC is enabled by a single compiler flag.

## Plain command line

```bash
g++ -O3 -fopenmp pi_openmp.cpp -o pi_openmp
```

- **`-fopenmp`** — both enables the OpenMP pragmas (otherwise they're silently ignored as comments) *and* links `libgomp` (GNU's OpenMP runtime).
- **`-O3`** — let the compiler inline and vectorise. Without optimisation, OpenMP timings are misleading.

## Verifying OpenMP is actually enabled

The most common silent failure is running an OpenMP program that was *compiled without* `-fopenmp`. The pragmas become no-ops; the program runs serially at full single-thread speed; nothing tells you anything is wrong.

Two ways to catch this:

1. The program prints `omp_get_max_threads()`. If it's 1, OpenMP didn't activate.
2. Log `OMP_DISPLAY_AFFINITY=TRUE`. With one thread, you'll see only one line.

## CMake — what the repo uses

```cmake
find_package(OpenMP REQUIRED COMPONENTS CXX)
add_executable(pi_openmp pi_openmp.cpp)
target_link_libraries(pi_openmp PRIVATE OpenMP::OpenMP_CXX)
```

`find_package(OpenMP REQUIRED)`:

- Detects which compiler you're using.
- Discovers the right OpenMP flag (`-fopenmp` for GCC, `-Xpreprocessor -fopenmp -lomp` for AppleClang+libomp, `/openmp` for MSVC, etc.).
- Configures `OpenMP::OpenMP_CXX` as an imported target carrying both compile and link flags.
- Fails configuration if OpenMP isn't available.

`target_link_libraries(... OpenMP::OpenMP_CXX)` propagates everything to the executable.

You don't write `-fopenmp` directly anywhere with CMake — it's all handled.

## On the course examples

The whole CMake configuration for OpenMP is two lines (`examples/openmp/CMakeLists.txt`):

```cmake
add_executable(pi_openmp pi_openmp.cpp)
target_link_libraries(pi_openmp PRIVATE OpenMP::OpenMP_CXX)
```

The top-level `examples/CMakeLists.txt` does the `find_package(OpenMP)` once for the whole project.

## On macOS

Apple Clang ships *without* OpenMP support by default. To build the OpenMP examples on a Mac, install Homebrew GCC:

```bash
brew install gcc
CXX=g++-14 cmake -B build ...
```

Or install `libomp` and configure CMake to use Apple Clang plus the Homebrew runtime — more fiddly and error-prone, so we recommend GCC on Mac for this course.

## Related

- [[OpenMP PBS Script]] — running the built binary on CX3.
- [[../examples/Building the Examples]] — full build instructions.
- [[OpenMP Pitfalls]] — the "silently disabled OpenMP" failure mode.
