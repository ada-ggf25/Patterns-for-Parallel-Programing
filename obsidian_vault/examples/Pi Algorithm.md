# π by Numerical Integration

The course's running example. Same algorithm in three implementations: serial, OpenMP, MPI.

## The mathematics

We approximate

$$
\pi = \int_0^1 \frac{4}{1 + x^2} \, dx
$$

by the **midpoint rule**: split $[0, 1]$ into $N$ equal-width sub-intervals, evaluate the integrand at the midpoint of each, and sum:

$$
\pi \approx \frac{1}{N} \sum_{i=1}^{N} \frac{4}{1 + \left(\frac{i - \tfrac{1}{2}}{N}\right)^2}
$$

For $N = 10^9$ this gives π correct to about 14 decimal places.

## Why this is the running example

The integral is **embarrassingly parallel**: every term in the sum is independent. There are no dependencies between iterations, so:

- The serial baseline is a single tight loop.
- The OpenMP version is one pragma — see [[pi_openmp]].
- The MPI version is a block partition + one collective — see [[pi_mpi]].

It's the cleanest possible parallel benchmark: anything beyond near-linear speedup is overhead, not work.

## Why it matters that this is the lucky case

Embarrassingly parallel problems are the **best case** for parallelisation. Most real codes have:

- Serial setup and I/O.
- Reductions that aren't pure sums.
- Inter-iteration dependencies that force ordering.
- Memory-bandwidth ceilings before compute saturates.

So the speedups you see for π — near-linear up to dozens of cores — are the upper bound, not the floor. See [[../openmp/Amdahls Law]] for the framework.

## Problem size

All three implementations use `N = 1'000'000'000LL` (one billion), hard-coded as:

```cpp
const long long n = 1'000'000'000LL;
```

To shorten for class demos or extend for benchmarking, edit this line in the source file. The value is suffixed `LL` (long long literal) because `1'000'000'000` overflows 32-bit int representations on some platforms.

## The three implementations

| File | Strategy | Key change vs serial |
|---|---|---|
| `examples/serial/pi_serial.cpp` | one loop | (baseline) |
| `examples/openmp/pi_openmp.cpp` | OpenMP | one `#pragma omp parallel for reduction(+:sum)` |
| `examples/mpi/pi_mpi.cpp` | MPI | block partition + `MPI_Reduce` |

The serial version uses `std::chrono::steady_clock`; the MPI version uses `MPI_Wtime()` (which is convenient because it's available everywhere MPI is). The OpenMP version uses chrono too.

## Related

- [[pi_serial]] — the baseline.
- [[pi_openmp]] — the OpenMP version.
- [[pi_mpi]] — the MPI version.
- [[Building the Examples]]
- [[Running on CX3]]
