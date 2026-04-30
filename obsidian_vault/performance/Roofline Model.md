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

The actual A3 kernel reads only the 6 face neighbours — the centre point is excluded:

```cpp
u_next[i,j,k] = (u[i-1,j,k] + u[i+1,j,k]
               + u[i,j-1,k] + u[i,j+1,k]
               + u[i,j,k-1] + u[i,j,k+1]) / 6.0;
```

- Reads: 6 doubles × 8 B = 48 B
- Writes: 1 double × 8 B = 8 B
- FLOPs: 5 adds + 1 divide = 6

Naive OI = 6 / 56 ≈ **0.11 FLOPs/byte** → bandwidth-bound, well below ridge at 18.7.
With cache reuse factored in: ~0.14 FLOPs/byte (still firmly bandwidth-bound).

> **Note:** Day 4 slides show the stencil as `(u[i,j,k] + 6 neighbours) / 7` (7 reads, 7 FLOPs, 64 B) for a pedagogical illustration. The actual A3 code omits the centre — do not include `u[i,j,k]` and do not divide by 7. Either model gives naive OI ≈ 0.11, so the conclusion is identical.

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

## Theoretical peak vs HPL-achievable: which ceiling to use?

Both numbers appear in the Rome table. They tell different stories:

| Ceiling | Value | What it means |
|---|---|---|
| Theoretical peak | 4608 GFLOPs | Every FMA unit fully pipelined, AVX2, 128 cores, all at once — never achieved by real code |
| HPL-achievable | 2896 GFLOPs (63 %) | The highest sustained DP throughput measured by vendor-tuned DGEMM (HPL) on this hardware |

**For non-DGEMM code** (including A1 and A2), the HPL ceiling is the more honest comparison. Your scalar integration kernel will land at a small fraction of 4608 GFLOPs — but framing it against theoretical peak makes the gap look larger than it is. HPL represents "the best real code actually achieves on this hardware", so `achieved / 2896` is the fairer roofline fraction.

Example: A1 achieves ~16 GFLOPs at 128T.
- vs theoretical: 16 / 4608 ≈ 0.35 % — looks tiny.
- vs HPL: 16 / 2896 ≈ 0.55 % — still low, but the ceiling is more realistic.

Both fractions should be reported in REFLECTION Section 3. The HPL fraction is the "honest" one.

## Top500 context

The [Top500](https://top500.org/) list ranks the world's largest supercomputers biannually (ISC in June, SC in November) since 1993 — all ranked by **Rmax** (HPL achieved throughput):

- Efficiency `Rmax / Rpeak` typically **65–85 %** across the list — the same ratio as our 63 % on Rome with OpenBLAS.
- Exascale era: leaders (Frontier, Aurora, El Capitan) at ≥ 1 EFLOPs ≈ 10⁶ × one Rome node.
- Sister lists: **HPCG** (memory-bound sparse benchmark; reorders the ranking vs HPL) and **Green500** (FLOPs/W).

HPL is deliberately used for Top500 because it represents the best-case sustained throughput achievable by vendor-tuned code; real workloads sit much lower.

## Limitations of the model

The roofline assumes a flat, average byte cost per kernel. It does not model:
- Cache reuse or prefetching
- NUMA placement
- Instruction-level parallelism

Two kernels with the same source-level OI can land at very different points once the memory hierarchy is involved. Treat roofline as a first-order ceiling, not a prediction.

## A3 roofline fraction

```
achieved_GBs   = bytes_moved / time_s / 1e9
               = (NX-2) × (NY-2) × (NZ-2) × NSTEPS × 56 / time_s / 1e9
               = 508³ × 100 × 56 / time_s / 1e9
               ≈ 734 / time_s   [GB/s for the standard A3 problem at NSTEPS=100]
ceiling_GBs    = STREAM_at_P_threads
fraction       = achieved_GBs / ceiling_GBs
```

56 B per update = **6 reads × 8 B + 1 write × 8 B** (actual stencil: 6 face neighbours, no centre).

> **Do not use the FLOPs/OI path here.** `FLOPs_total / OI / time_s / 1e9` with OI = 0.14 and 7 FLOPs/update gives ≈ 663/time_s — about 10 % below the correct 734/time_s. The discrepancy comes from mixing the 7-read pedagogical model in the slides (7 FLOPs / 50 effective bytes = 0.14) with the actual 6-read stencil (56 bytes/update). Use direct byte counting. See [[../assessment/A3 Jacobi]] for the explicit formula.

Graduated A3 thresholds: ≥ 0.70 → full marks; ≥ 0.50 → 70%; ≥ 0.30 → 40%; ≥ 0.15 → 15%; else 0.

## Related

- [[STREAM and HPL]] — how the ceilings are measured.
- [[Performance Metrics]] — reference-parallel-time vs roofline fraction.
- [[NUMA First Touch]] — affects the effective bandwidth fraction.
- [[../assessment/A3 Jacobi]] — A3 is scored on roofline fraction.
