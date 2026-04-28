# NUMA First Touch

Linux places a memory page on the NUMA node of the thread that **first writes** to it, not the thread that calls `malloc`/`new`/`std::vector::resize`. This is the **first-touch policy**.

## Why it matters on Rome

Rome has 8 NUMA domains (4 per socket). Cross-socket memory access costs ~250 ns vs ~80 ns for local DRAM — roughly 3×. For a bandwidth-bound kernel like A3 Jacobi, NUMA placement is the dominant performance factor.

| Access | Approx latency on Rome |
|---|---|
| L1 hit | 1 ns |
| L2 hit | 4 ns |
| L3 hit (same chiplet) | ~15 ns |
| Local DRAM | ~80 ns |
| Cross-socket DRAM | ~250 ns |

## The naive mistake: single-threaded init

```cpp
std::vector<double> u(NX * NY * NZ);

// BUG: master thread first-touches ALL pages → all on socket 0
for (size_t i = 0; i < u.size(); ++i)
    u[i] = 0.0;

// 128 threads read from u — 7/8 of accesses cross the NUMA fabric
#pragma omp parallel for
for (size_t i = 0; i < u.size(); ++i)
    u[i] = stencil(u, i);
```

All pages land on socket 0 (master's NUMA domain). 7/8 of thread accesses are cross-socket.

## The fix: parallel first-touch init

```cpp
std::vector<double> u(NX * NY * NZ);

// Each thread first-touches its slice → pages distribute across 8 domains
#pragma omp parallel for default(none) shared(u)
for (size_t i = 0; i < u.size(); ++i)
    u[i] = static_cast<double>(i);
```

The parallel-for traversal means each thread writes the slice it will later read — pages are pinned to each thread's NUMA node automatically.

## The matching compute kernel

The consumer must use the **same traversal pattern** as the init:

```cpp
double sum_parallel(const std::vector<double>& v)
{
    double s = 0.0;
#pragma omp parallel for default(none) shared(v) reduction(+:s)
    for (std::size_t i = 0; i < v.size(); ++i)
        s += v[i];   // each thread reads the pages it initialised
    return s;
}
```

If the consumer uses a different schedule or different chunk boundaries, threads end up reading pages they didn't first-touch. Match the access pattern, not just the loop bounds.

## Measured impact on Rome

Parallel-init / parallel-compute vs serial-init / parallel-compute at 128 threads: typically **3–5× faster** on a full dual-socket node. The effect is negligible on a single-socket run (only 1 NUMA fabric boundary to cross).

## STREAM bandwidth and more threads

Counter-intuitive measurement from CX3:

| Threads | STREAM Triad GB/s |
|---|---:|
| 32 (spread, 1 per CCX) | **246** |
| 64 (spread) | 237 |
| 128 (full node) | 231 |
| 32 (close, 1 socket) | 116 |

32 threads beat 128 by ~6 %. DRAM bandwidth is fixed; it's saturated by ~1 thread per CCX. Adding more threads causes L3 contention with no extra bandwidth to win.

## `numactl` for testing

```bash
numactl --cpunodebind=0 --membind=0 ./stencil    # pin all to NUMA node 0
numactl --interleave=all ./stencil                # round-robin pages across all nodes
```

Requires loading `OpenMPI` module (`ml OpenMPI`) to get `numactl` on PATH.

## A3 extension

A3's `numa_first_touch` extension: swap naive init for parallel-init; measure the delta. Soft threshold: `delta_percent ≥ 15` → full marks; `≥ 5` → half marks.

## Related

- [[False Sharing]] — the other cache-line layout issue.
- [[Roofline Model]] — bandwidth ceiling affected by NUMA placement.
- [[../cluster/AMD Rome Architecture]] — Rome's 8-domain topology.
- [[../assessment/A3 Jacobi]] — A3-core requires parallel init; extension measures the delta.
