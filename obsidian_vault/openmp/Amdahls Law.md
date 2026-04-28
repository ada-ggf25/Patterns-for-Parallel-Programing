# Amdahl's Law

The ceiling on speedup from parallelisation. If a fraction $B$ of the program is inherently serial, the maximum speedup with $N$ cores is

$$
\mathrm{Speedup}(N) = \frac{1}{B + (1-B)/N}
$$

## What it tells you

| Serial fraction $B$ | Speedup at N=∞ | Speedup at N=8 | Speedup at N=64 |
|---|---:|---:|---:|
| 0 (perfectly parallel) | ∞ | 8.0 | 64.0 |
| 0.01 (1% serial) | 100 | 7.5 | 39.3 |
| 0.05 (5% serial) | 20 | 5.9 | 15.4 |
| 0.10 (10% serial) | 10 | 4.7 | 9.0 |
| 0.50 (50% serial) | 2 | 1.8 | 2.0 |

Two takeaways:

- **Even tiny serial fractions kill scaling.** 1 % serial limits you to 100× — no matter how many cores you throw at it.
- **For parallelisation to be worth the engineering effort, the parallel fraction must dominate.** If most of your program is serial, consider profiling and parallelising the parallel-friendly bits *first* before going wide.

## π by integration is the lucky case

Our running example is *embarrassingly parallel*: every term in the sum is independent, so $B \approx 0$. That's why we expect close-to-linear speedup. Most real codes aren't this clean — they have serial setup, I/O, reductions, synchronisation, dependencies between loops — and serial fraction grows.

## Why π still doesn't quite hit linear speedup

Even with $B \approx 0$ in theory, you don't see exactly 8× from 8 threads. Reasons:

- **Fork-join overhead** — creating, dispatching, and joining threads costs microseconds. For a 0.16 s loop, that overhead is small but nonzero.
- **Reduction cost** — combining 8 partial sums takes work proportional to log N.
- **Vectorisation** — the serial code may auto-vectorise differently from the OpenMP version.
- **CPU frequency** — Turbo Boost gives single-threaded code higher clocks than 8-threaded code. The serial baseline is artificially fast.
- **NUMA / cache** — threads scattered across NUMA domains pay extra for any shared data.

For the course's `pi_openmp` on 8 threads on Rome, expect roughly **6–7× speedup** vs serial — not 8×.

## Beyond Amdahl

Amdahl's law assumes a fixed problem size. **Gustafson's law** is the complementary view: as you add cores, you typically *grow* the problem to keep the parallel work dominant. Many real HPC codes operate in the Gustafson regime — strong scaling looks bad but weak scaling (problem size scales with cores) looks fine.

For benchmark reporting, distinguish:

- **Strong scaling** — fixed problem, more cores. What Amdahl describes.
- **Weak scaling** — problem grows with cores. What Gustafson describes.

## Related

- [[OpenMP Overview]] — the parallelism model this constrains.
- [[../examples/Pi Algorithm]] — the embarrassingly parallel example.
- [[../mpi/MPI Overview]] — when you outgrow one node and need MPI.
