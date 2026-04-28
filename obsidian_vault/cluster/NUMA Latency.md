# NUMA Latency

The cost of a DRAM access on a Rome node depends on which NUMA domain you're reading from. The `examples/benchmarks/numa_latency/` benchmark in the repo measures this directly with a pointer-chase probe.

## The measurement

For each (src, tgt) pair of NUMA domains:

1. Pin the CPU to NUMA domain `src` with `numactl --cpunodebind=src`.
2. Allocate a 256 MB buffer on NUMA domain `tgt` with `numa_alloc_onnode`.
3. Initialise it as a single-cycle Sattolo permutation (so `p = buf[p]` visits every slot before returning).
4. Warm up, then run 50M dereferences and report nanoseconds per access.

256 MB is well beyond the 16 MB per-CCX L3 on Rome, so the benchmark is firmly in DRAM territory. See [[../examples/numa_latency benchmark]] for the source-code walk-through.

## Measured matrix on `cx3-13-2`

ns per DRAM access (diagonal = local):

| src \ tgt |   0 |   1 |   2 |   3 |   4 |   5 |   6 |   7 |
|:---:|----:|----:|----:|----:|----:|----:|----:|----:|
| **0** | **104** | 117 | 126 | 129 | 250 | 249 | 246 | 244 |
| **1** | 116 | **104** | 129 | 125 | 251 | 253 | 249 | 248 |
| **2** | 123 | 129 | **104** | 117 | 243 | 243 | 246 | 247 |
| **3** | 128 | 124 | 118 | **103** | 249 | 250 | 251 | 252 |
| **4** | 246 | 249 | 243 | 245 | **103** | 117 | 125 | 128 |
| **5** | 249 | 250 | 249 | 249 | 116 | **105** | 128 | 123 |
| **6** | 243 | 244 | 247 | 247 | 125 | 129 | **104** | 116 |
| **7** | 246 | 248 | 251 | 252 | 129 | 125 | 118 | **104** |

## Three tiers of cost

- **Local** (diagonal): ~104 ns
- **Same-socket, adjacent NUMA** (0↔1, 2↔3, 4↔5, 6↔7): ~117 ns
- **Same-socket, non-adjacent**: ~126 ns
- **Cross-socket** (over xGMI, NUMA 0–3 ↔ NUMA 4–7): ~247 ns

## SLIT vs reality

The BIOS-reported SLIT values are 10 / 12 / 32 (local / same-socket / cross-socket). Measured ratios are 1.00 / 1.12–1.24 / 2.37. SLIT orders the tiers correctly but:

- **Overstates cross-socket cost** (predicts 3.2× local; it's actually 2.37×).
- **Hides the ~10 % intra-socket structure** — same SLIT "12" covers both 117 and 129 ns pairs.

Treat SLIT as a hint, not as a calibrated measurement.

## What this means for placement

- The cost difference between local and cross-socket is ~2.4×. For a memory-bandwidth-bound kernel that re-reads its working set, that's roughly the speedup you can win or lose by getting placement right.
- Same-socket accesses are within 25 % of local — if you can keep threads on the same socket, getting NUMA placement *exactly* right is a smaller win than avoiding cross-socket traffic entirely.
- The xGMI link bandwidth is shared by every cross-socket transaction, so contention can make the cross-socket penalty larger than these single-stream numbers suggest.

## Related

- [[AMD Rome Architecture]] — the topology these numbers live in.
- [[Topology Inspection]] — see your NUMA layout.
- [[../openmp/Thread Pinning]] — using this knowledge in OpenMP.
- [[../examples/numa_latency benchmark]] — the source code that produced these numbers.
