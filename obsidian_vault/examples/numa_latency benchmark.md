# `numa_latency` Benchmark

A small C program in `examples/benchmarks/numa_latency/` that measures the per-NUMA-domain DRAM latency on a Rome compute node by pointer-chasing through a buffer pinned to a specific NUMA domain.

## Why this exists

The course slides quote a measured NUMA-latency matrix on Rome (~104 ns local, ~117 ns same-socket adjacent, ~247 ns cross-socket — see [[../cluster/NUMA Latency]]). This benchmark is the source of those numbers; it lets you reproduce or update them yourself.

## Source — `numa_latency.c`

```c
// numa_latency.c — NUMA-aware pointer-chase latency probe.
//
// Allocates a buffer on the NUMA node passed as argv[1] and runs a
// serialised pointer chase over it; the average dereference time is
// printed as lat_ns=<ns>.
//
// Usage (run via numactl so the *CPU* side is pinned):
//   numactl --cpunodebind=<src> ./numa_latency <tgt> [MB] [iters]
```

The core idea:

1. **Allocate** a `<MB>`-megabyte buffer on NUMA node `<tgt>` with `numa_alloc_onnode`.
2. **Initialise** it with **Sattolo's algorithm** — a single-cycle permutation of the indices, so `p = buf[p]` visits every slot once before returning to start. This defeats prefetchers and ensures every access actually goes to DRAM.
3. **Warm up** by chasing for 2M iterations to fault in pages and settle the chase.
4. **Time** 50M dereferences and report ns/access.

Default buffer is 256 MB — well beyond Rome's 16 MB per-CCX L3, so the working set spills firmly into DRAM.

## The `numactl` invocation

```bash
numactl --cpunodebind=<src> --localalloc -- ./numa_latency <tgt> 256 50000000
```

- `--cpunodebind=<src>` pins the **CPU** to NUMA node `src`.
- `numa_alloc_onnode(...)` inside the program pins the **memory** to NUMA node `tgt`.
- `--localalloc` ensures any *other* allocations (small bookkeeping) come from `src`'s memory, not random NUMAs.

The two pinnings combined define a (src, tgt) latency cell.

## Driving script — `numa_latency.pbs`

```bash
#PBS -N numa_latency
#PBS -l walltime=00:45:00
#PBS -l select=1:ncpus=128:mem=500gb:cpu_type=rome
```

Asks for an **entire Rome node** (`ncpus=128`) so the script can iterate over all 8 NUMA domains as both source and target. Builds `numa_latency` with `gcc -O2 ... -lnuma`, then runs the 8×8 matrix of measurements.

The output is a tab-separated 8×8 table — see [[../cluster/NUMA Latency]] for the actual numbers.

## What you can do with it

- **Confirm Rome's NPS4 layout.** Compare the matrix structure (4 quadrants of 4) to what's expected.
- **Detect contention.** Re-run while a co-tenant job is loading the cross-socket link; cross-socket numbers should rise.
- **Compare hardware.** Swap `cpu_type=rome` for `cpu_type=icelake` (and shrink `ncpus` to 64); you'll get a 2×2 matrix with very different cross-socket cost.
- **Tune buffer size.** Drop to 32 MB and you'll see cache effects intrude; drop to 4 MB and you'll see L3-only numbers.

## Related

- [[../cluster/NUMA Latency]] — the measured matrix.
- [[../cluster/AMD Rome Architecture]] — what NPS4 means.
- [[../cluster/Topology Inspection]] — finding your NUMA layout.
- [[../openmp/Thread Pinning]] — why these numbers matter for OpenMP placement.
