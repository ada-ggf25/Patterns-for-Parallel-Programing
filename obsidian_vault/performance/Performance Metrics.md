# Performance Metrics

Four distinct metrics answer different questions about parallel performance. Knowing which to report — and why — matters for the assessments.

## The four metrics

| Metric | Formula | Question answered |
|---|---|---|
| Time-to-solution | T(P) | How long does the user wait? |
| Speedup | T(1) / T(P) | How much faster did parallelism make *this code*? |
| Reference-parallel-time ratio | T_ref(P) / T(P) | How does *my code* compare to a reference implementation? |
| Roofline fraction | achieved / ceiling | How close am I to the hardware limit? |

## Speedup — headline but misleading

Speedup is the number people quote, but it can be gamed:

| Student | T(1) | T(128) | Self-speedup | Wall clock |
|---|---|---|---|---|
| A | 100 s | 1.0 s | **100×** | 1.0 s |
| B | 8 s | 0.8 s | 10× | 0.8 s |

Student A's "100× speedup" looks heroic, but student B's code is *faster in wall clock*. The big speedup comes from a slow serial baseline, not a good parallel implementation.

## Reference-parallel-time ratio (A1, A2 grading)

$$\text{ratio} = \frac{T_{\text{ref}}(128)}{T_{\text{student}}(128)} \qquad \text{(capped at 1.0 for scoring)}$$

Normalises out the quality of the serial baseline. A ratio of 1.0 means you're as fast as the instructor's reference at 128 threads. A ratio > 1.0 means faster (capped at 1.0 for grade purposes).

The reference time `T_ref` is published once at the start of the cohort and frozen.

## Efficiency

$$\text{efficiency} = \frac{\text{speedup}}{P} = \frac{T(1)}{P \cdot T(P)}$$

Ideal efficiency = 1.0 (linear scaling). In practice efficiency drops with P due to Amdahl's law, synchronisation, and communication overhead.

**tables.csv check:** the grader verifies `speedup = T(1)/T(P)` and `efficiency = speedup/P` within 2 % for each row. Both quantities are internal-consistency checks only — no cross-check against the instructor's timings.

## Roofline fraction (A3 grading)

$$\text{fraction} = \frac{\text{achieved GB/s at P threads}}{\text{STREAM ceiling at P threads}}$$

For A3 Jacobi (bandwidth-bound), the ceiling is the measured STREAM Triad at the same thread count. Graduated thresholds: ≥ 0.70 → full; ≥ 0.50 → 70%; ≥ 0.30 → 40%; ≥ 0.15 → 15%; else 0.

## The bad-serial trap

A slow serial baseline gives a misleadingly high speedup:
- Missed optimisations in the serial code inflate T(1).
- Any parallel implementation looks like a supercomputer.

Reference-parallel-time scoring specifically closes this loophole: if your parallel code is slow, you score low — regardless of how bad the serial baseline was.

## Related

- [[Roofline Model]] — ceiling computation for A3.
- [[Timing omp_get_wtime]] — how to measure T(P) correctly.
- [[../openmp/Amdahls Law]] — why efficiency drops with P.
- [[../assessment/Assessment Overview]] — rubric and grading formulas.
