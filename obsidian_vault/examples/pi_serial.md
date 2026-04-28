# `pi_serial.cpp` — The Baseline

A plain C++ implementation of the [[Pi Algorithm|π midpoint integration]]. No threading, no MPI — the reference timing.

## The full file

```cpp
#include <chrono>
#include <cstdio>

int main() {
    const long long n = 1'000'000'000LL;
    const double w = 1.0 / static_cast<double>(n);
    double sum = 0.0;

    const auto t0 = std::chrono::steady_clock::now();

    for (long long i = 1; i <= n; ++i) {
        const double x = w * (static_cast<double>(i) - 0.5);
        sum += 4.0 / (1.0 + x * x);
    }

    const double pi = w * sum;
    const auto t1 = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(t1 - t0).count();

    std::printf("n=%lld  pi=%.15f  time=%.3fs\n", n, pi, secs);
    return 0;
}
```

## Annotations

- **`const long long n = 1'000'000'000LL`** — one billion iterations. The `LL` suffix forces a `long long` literal (avoids overflow on 32-bit-int platforms).
- **`const double w = 1.0 / n`** — the width of each midpoint sub-interval. Computed once, hoisted out of the loop.
- **The loop body** — for each `i`, compute the midpoint `x = w * (i - 0.5)`, evaluate `4 / (1 + x²)`, accumulate.
- **`std::chrono::steady_clock`** — wall-clock timing that's monotonic and isn't affected by NTP adjustments. Better than `time()` for benchmarking.
- **`%.15f`** — print 15 decimal places, enough to see all the precision a `double` carries.

## Building

```bash
ml tools/prod GCC                # or GCC CMake to use the project's CMakeLists
g++ -O3 pi_serial.cpp -o pi_serial
```

Or with CMake (what the repo uses):

```bash
cd ic-hpc-intro/examples
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target pi_serial
./build/serial/pi_serial
```

## Expected output

```
n=1000000000  pi=3.141592653589821  time=10.234s
```

(Wall-time varies — ~10 s on a Rome compute node, more on slower hardware.)

## Submitting on CX3

```bash
qsub serial/pi_serial.pbs
```

The PBS script for this is minimal — see `examples/serial/pi_serial.pbs`:

```bash
#!/bin/bash
#PBS -N pi_serial
#PBS -l walltime=00:10:00
#PBS -l select=1:ncpus=1:mem=4gb:cpu_type=rome

cd $PBS_O_WORKDIR
ml tools/prod
ml GCC

./build/serial/pi_serial
```

One core, 4 GB, 10 minutes, on Rome. See [[../pbs/Resource Selection]] for the directive grammar.

## Why use this as the baseline

Speedup is meaningful only against a fixed reference. The serial version *with the same compiler flags and the same n* gives you the denominator for "how much faster is OpenMP/MPI?".

Always run the serial version first, on the same `cpu_type` you'll use for the parallel runs. Don't compare a serial run on icelake against a parallel run on rome — you're conflating two different effects.

## Related

- [[Pi Algorithm]] — the maths.
- [[pi_openmp]] — the OpenMP variant.
- [[pi_mpi]] — the MPI variant.
- [[Building the Examples]]
