# STREAM and HPL Benchmarks

Two benchmarks that measure the two ceilings in the roofline model: STREAM for memory bandwidth, HPL for compute throughput.

## STREAM — memory bandwidth benchmark

STREAM measures four micro-kernels. Each stresses the memory subsystem differently:

| Kernel | Pattern | FLOPs | Rome CX3 @ 32T GB/s |
|---|---|---|---:|
| `Copy` | `c[i] = a[i]` | 0 | 337 |
| `Scale` | `b[i] = q*c[i]` | 1 | 224 |
| `Add` | `c[i] = a[i] + b[i]` | 1 | 245 |
| **`Triad`** | **`a[i] = b[i] + q*c[i]`** | **2 (FMA)** | **246** |

### Why Triad for roofline

- Closest access pattern to typical scientific kernels (mixed read/write + compute).
- Standard in the literature (McCalpin, Williams/Waterman/Patterson roofline paper, LIKWID).
- Honest ceiling: `Copy` at 337 GB/s would inflate the ceiling and unfairly penalise A3 students.

### Rome measured values (CX3, 2026-04-26)

| Threads | Triad GB/s |
|---|---:|
| 32 (spread, 1 per CCX) | **246.2** |
| 64 | 237 |
| 128 (full node) | 231.5 |
| 32 (close, 1 socket) | 116.0 |

`STREAM_ARRAY_SIZE=800M`, both GCC 13.3 and Clang 18 (compilers tied).

Counter-intuitive: 32 threads beat 128 by ~6 %. Rome has 32 CCXs; DRAM bandwidth is fixed and saturated by ~1 thread per CCX. Extra threads cause L3 contention with no gain.

## HPL — compute throughput benchmark

HPL (High-Performance Linpack) solves a random dense `N × N` system by LU factorisation and reports throughput in GFLOPs. It is the benchmark behind the **Top500** supercomputer ranking.

- ≈ 90 % of FLOPs land in DGEMM (the rank-k updates in LU).
- DGEMM is compute-bound (very high OI), so the FPU — not memory — sets the ceiling.
- HPL is the "achievable peak" — what vendor BLAS can squeeze out under optimal conditions.

### Rome measured value (CX3, 2026-04-26)

| Parameter | Value |
|---|---|
| BLAS | OpenBLAS 0.3.27 (Icelake-tuned) |
| Layout | 8 MPI ranks × 16 OpenMP threads |
| Problem size | N = 80 000 (≈ 51 GB matrix) |
| **Achieved DP** | **2896 GFLOPs ≈ 63 % of theoretical 4608** |

The 12-point gap to AMD's published ~75 % is the BLAS choice: AOCL-BLIS (Zen-tuned) would close most of it.

## What this means for assessments

- **A1 + A2** are compute-bound in principle but are not DGEMM. Branchy iterations, spike regions, and limited reuse mean they achieve a few percent of HPL at best. Graded on reference-parallel-time ratio, not raw GFLOPs.
- **A3** is memory-bound (OI ≈ 0.14). STREAM is the relevant ceiling. Graded on roofline fraction.

## Related

- [[Roofline Model]] — how STREAM and HPL ceilings combine.
- [[NUMA First Touch]] — affects effective STREAM fraction.
- [[Performance Metrics]] — reference-parallel-time and roofline fraction.
