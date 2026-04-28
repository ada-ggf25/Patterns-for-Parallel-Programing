# Assessment Overview

Three parallel-programming assessments graded on a 100-point scale. All three assessments open on day 2 and the final snapshot is taken at the end of day 5.

## 100-pt rubric

| Component | Pts |
|---|---|
| A1 — Numerical integration | 20 |
| A2 — Mandelbrot two-variant comparison | 30 |
| A3-core — 3D Jacobi stencil | 25 |
| A3-extension — chosen branch | 15 |
| Hygiene (build clean + lint + README) | 10 |
| **Total** | **100** |

Commit-history quality contributes 0 pts but is reviewed for academic integrity.

## Canonical thread ladder

All performance measurements use: **{1, 16, 64, 128}**

This maps directly to Rome's geometry: serial / one NUMA domain (16 cores each) / one socket (64 cores) / full dual-socket node (128 cores).

## Common scoring formula

**A1 + A2** (compute-bound): reference-parallel-time ratio at 128 threads.

$$\text{score} = \min\!\left(1.0,\; \frac{T_{\text{ref}}(128)}{T_{\text{student}}(128)}\right) \times \text{full\_pts}$$

**A3-core** (bandwidth-bound): roofline fraction.

$$\text{score} = \text{graduated}(\text{achieved\_GBs} / \text{STREAM\_ceiling\_GBs})$$
Graduated: ≥ 0.70 → full; ≥ 0.50 → 70%; ≥ 0.30 → 40%; ≥ 0.15 → 15%; else 0.

Both are **correctness-gated**: any thread-count correctness failure → 0 performance score.

## `tables.csv` internal consistency

Every assessment requires a `tables.csv` with times, speedups, and efficiencies:

$$\text{speedup} = T(1) / T(P) \qquad \text{efficiency} = \text{speedup} / P$$

The grader checks these arithmetic identities within 2 %. This is an internal-consistency check only — your own measurements vs your own derived columns. No cross-check against instructor re-runs.

## Hygiene (10 pts)

| Component | Pts | Check |
|---|---|---|
| Build clean (no warnings at `-Wall -Wextra -Wpedantic`) | 3 | Compiler output |
| Lint (clang-format, clang-tidy, cppcheck) | 4 | CI lint workflow |
| README / English readability | 3 | Manual |

## CI / GitHub Actions

Push to your branch to get formative feedback:
- `snippets-build.yml` — build + correctness tests + TSan+Archer (Ubuntu)
- `snippets-macos.yml` — build + tests (macOS)
- `lint-cpp.yml` — clang-format
- `lint-quarto.yml` — markdownlint + link check

Green CI is a necessary but not sufficient condition for full marks.

## Hardware (Rome, CX3)

- Dual-socket EPYC 7742 = 128 physical cores, 8 NUMA domains, SMT off.
- Peak DP: 4608 GFLOPs (theoretical). HPL achieved: 2896 GFLOPs ≈ 63 %.
- STREAM Triad: 246.2 GB/s (32T, 1 per CCX); 231.5 GB/s (128T full node).
- Ridge OI: 4608 / 246 ≈ 18.7 FLOPs/byte.

## Related

- [[A1 Integration]] — 20 pts, parallel-for + reduction + schedule analysis.
- [[A2 Mandelbrot]] — 30 pts, parallel-for vs tasks.
- [[A3 Jacobi]] — 40 pts, bandwidth-bound stencil + extension.
- [[../performance/Performance Metrics]] — speedup, roofline fraction, reference-parallel-time.
- [[../performance/Roofline Model]] — ceiling computations.
