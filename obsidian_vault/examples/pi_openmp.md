# `pi_openmp.cpp` — The OpenMP Version

The OpenMP version differs from the [[pi_serial|serial baseline]] by exactly **one line**: the `#pragma omp parallel for reduction(+:sum)` above the loop.

## The full file

```cpp
#include <chrono>
#include <cstdio>
#include <omp.h>

int main() {
    const long long n = 1'000'000'000LL;
    const double w = 1.0 / static_cast<double>(n);
    double sum = 0.0;

    const auto t0 = std::chrono::steady_clock::now();

    #pragma omp parallel for reduction(+:sum)         // <-- only addition vs serial
    for (long long i = 1; i <= n; ++i) {
        const double x = w * (static_cast<double>(i) - 0.5);
        sum += 4.0 / (1.0 + x * x);
    }

    const double pi = w * sum;
    const auto t1 = std::chrono::steady_clock::now();
    const double secs = std::chrono::duration<double>(t1 - t0).count();

    std::printf("n=%lld  threads=%d  pi=%.15f  time=%.3fs\n",
                n, omp_get_max_threads(), pi, secs);
    return 0;
}
```

## What the one line does

`#pragma omp parallel for reduction(+:sum)` does three things at once:

1. **`parallel`** — fork a team of threads (count = `OMP_NUM_THREADS`).
2. **`for`** — split the loop iterations across the team.
3. **`reduction(+:sum)`** — give each thread a private `sum`; combine them with `+` at the end.

Without `reduction(+:sum)`, the loop has a race condition on the shared `sum` and produces nondeterministic, almost-always-wrong answers. See [[../openmp/reduction clause]] and [[../openmp/Variable Scoping]].

## Why no other change is needed

The loop is canonical (single integer counter, constant bounds, simple step), the body has no inter-iteration dependencies, and the only shared state is the accumulator `sum` — which we've handled. So one pragma covers everything.

This is the lucky case. Real codes usually need additional `private`, `firstprivate`, or scope-tightening to avoid races. See [[../openmp/OpenMP Pitfalls]].

## `omp_get_max_threads()` in the printf

Reports the team size that was active for the most recent parallel region. If you forgot `-fopenmp` at compile time, this prints `1` regardless of `OMP_NUM_THREADS` — a quick sanity check that OpenMP actually activated.

## Building

```bash
g++ -O3 -fopenmp pi_openmp.cpp -o pi_openmp
```

Or with CMake (what the repo uses):

```bash
cd ic-hpc-intro/examples
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target pi_openmp
OMP_NUM_THREADS=8 ./build/openmp/pi_openmp
```

The `find_package(OpenMP REQUIRED)` call in the parent CMakeLists.txt arranges the `-fopenmp` flag for you. See [[../openmp/Building OpenMP]].

## Running on CX3

```bash
qsub openmp/pi_openmp.pbs
```

For the PBS script and what to expect from the output, see [[../openmp/OpenMP PBS Script]] and [[../openmp/Reading the OpenMP log]].

## Expected scaling

| Threads | Wall-time (Rome) | Speedup vs serial |
|---|---:|---:|
| 1 | ~10 s | 1.0× |
| 4 | ~2.5 s | 4.0× |
| 8 | ~1.4 s | 7.1× |
| 16 | ~0.8 s | 12× |
| 64 | ~0.25 s | 40× |
| 128 | ~0.15 s | 67× |

Falls off from linear at high thread counts due to fork-join overhead, reduction cost, and CPU frequency drop under load — see [[../openmp/Amdahls Law]].

## Related

- [[Pi Algorithm]]
- [[pi_serial]] — the baseline.
- [[../openmp/parallel directive]] / [[../openmp/for directive]] / [[../openmp/reduction clause]]
- [[../openmp/OpenMP PBS Script]]
- [[../openmp/Reading the OpenMP log]]
