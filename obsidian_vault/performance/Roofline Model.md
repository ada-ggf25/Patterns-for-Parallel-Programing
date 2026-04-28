# Roofline Model

The roofline model frames kernel performance as bounded by one of two ceilings: compute throughput or memory bandwidth. It tells you *which* ceiling you're hitting and by how much.

## Operational intensity

$$OI = \frac{\text{FLOPs performed}}{\text{bytes moved to/from main memory}}$$

The performance ceiling:

$$\text{ceiling} = \min(\text{peak FLOPs/s},\; OI \times \text{STREAM bandwidth})$$

- **High OI** → compute-bound → aim for peak FLOPs.
- **Low OI** → bandwidth-bound → aim for STREAM bandwidth.

The **ridge point** separates the two regimes: `ridge_OI = peak_GFLOPs / STREAM_GBs`.

## Rome numbers (CX3, measured 2026-04-24)

| Quantity | Value |
|---|---|
| Peak DP GFLOPs (theoretical, full node) | 4608 (128 cores × 2.25 GHz × 16 FLOPs/cycle AVX2 FMA) |
| HPL achieved DP | 2896 GFLOPs ≈ 63 % of theoretical |
| STREAM Triad (32 threads, 1 per CCX) | **246.2 GB/s** |
| STREAM Triad (128 threads, full node) | 231.5 GB/s |
| Ridge OI | 4608 / 246 ≈ **18.7 FLOPs/byte** |

## OI for the Jacobi stencil (A3)

```cpp
u_next[i,j,k] = (u[i,j,k] + u[i±1,j,k] + u[i,j±1,k] + u[i,j,k±1]) * (1/7);
```

- Reads: 7 doubles × 8 B = 56 B
- Writes: 1 double × 8 B = 8 B
- FLOPs: 6 adds + 1 multiply = 7

Naive OI = 7 / 64 ≈ **0.11 FLOPs/byte** → bandwidth-bound, well below ridge at 18.7.
With cache reuse factored in: ~0.14 FLOPs/byte (still firmly bandwidth-bound).

## Assessment kernels

| Kernel | OI | Regime | Metric |
|---|---|---|---|
| A1 integration | >> 10 | Compute-bound | Reference-parallel-time ratio |
| A2 Mandelbrot | >> 10 | Compute-bound | Reference-parallel-time ratio |
| A3 Jacobi | ~0.14 | Memory-bound | Roofline fraction (achieved GB/s / STREAM ceiling) |

## Roofline ceiling helper (code)

```cpp
struct RooflineEstimate { double ceiling_gflops; bool compute_bound; };

RooflineEstimate roofline_ceiling(double oi, double peak_gflops, double bw_gbs)
{
    const double bw_ceiling = oi * bw_gbs;
    if (bw_ceiling < peak_gflops)
        return {bw_ceiling, false};   // memory-bound
    return {peak_gflops, true};       // compute-bound
}

// Rome: 1 thread per CCX ceiling
RooflineEstimate rome_ceiling(double oi) {
    return roofline_ceiling(oi, 4608.0, 246.2);
}
```

## Limitations of the model

The roofline assumes a flat, average byte cost per kernel. It does not model:
- Cache reuse or prefetching
- NUMA placement
- Instruction-level parallelism

Two kernels with the same source-level OI can land at very different points once the memory hierarchy is involved. Treat roofline as a first-order ceiling, not a prediction.

## A3 roofline fraction

```
achieved_GBs   = (bytes_moved_per_problem) / time_s / 1e9
ceiling_GBs    = STREAM_at_P_threads
fraction       = achieved_GBs / ceiling_GBs
```

Graduated A3 thresholds: ≥ 0.70 → full marks; ≥ 0.50 → 70%; ≥ 0.30 → 40%; ≥ 0.15 → 15%; else 0.

## Related

- [[STREAM and HPL]] — how the ceilings are measured.
- [[Performance Metrics]] — reference-parallel-time vs roofline fraction.
- [[NUMA First Touch]] — affects the effective bandwidth fraction.
- [[../assessment/A3 Jacobi]] — A3 is scored on roofline fraction.
