# AMD Rome Architecture

CX3's larger compute tier uses **AMD EPYC 7742** processors (codename Rome, Zen2 microarchitecture). Understanding the chip's hierarchy is essential for placing threads and ranks well — see [[../openmp/Thread Pinning|Thread Pinning]] and [[NUMA Latency]].

## Per-node tally

- **2 sockets × 8 chiplets (CCDs) × 2 Core Complexes (CCX) × 4 cores = 128 cores per node.**
- ~1 TB DDR4 RAM per node, split across the two sockets.
- Sockets are joined by three xGMI inter-socket links (~96 GB/s theoretical, 63–72 GB/s sustained per direction).
- Each socket has its own I/O die owning that socket's 8 memory controllers (204.8 GB/s per socket); DRAM on the other socket is reachable only over the xGMI links.
- ×16 PCIe 4.0 link to the InfiniBand adapter for inter-node MPI.

## Cache hierarchy

| Level | Size | Sharing |
|---|---|---|
| L1d | 32 KB | per core |
| L2 | 512 KB | per core |
| L3 | 16 MB | per CCX (4 cores) |

**There is no node-wide last-level cache.** The L3 is the sharing boundary. Four cores in the same CCX share 16 MB; four cores in different CCXs share nothing in cache.

This has a direct performance consequence: a 4-thread team that fits inside one CCX behaves very differently from one scattered across four CCXs. The first hits warm L3; the second misses to DRAM constantly.

## NUMA layout — NPS4 mode

The BIOS is configured in **NPS4** ("NUMA per socket = 4"), which gives **8 NUMA domains per node** (4 per socket). Each NUMA domain spans 2 CCDs.

Note: NPS4 does not give one NUMA domain per CCX (which would be 16). `numactl --hardware` is the authoritative report — see [[Topology Inspection]].

## NUMA distances

From `numactl --hardware` on a Rome compute node:

```
            node 0   node 1 (same socket)   node 4 (other socket)
            ------   --------------------   ---------------------
distance:    10               12                     32
```

These SLIT values are unitless BIOS hints — local is defined as 10 and other tiers scale from there. They orient you correctly but understate the cross-socket cost; see [[NUMA Latency]] for measured nanoseconds.

## Practical implications

- **Threads that share data:** keep them in the same NUMA domain (or at least the same socket). With OpenMP that means `OMP_PROC_BIND=close` and a tight `OMP_PLACES`. See [[../openmp/Thread Pinning]].
- **Bandwidth-bound kernels:** spread threads across sockets so they hit different memory controllers. `OMP_PROC_BIND=spread`.
- **MPI ranks:** one rank per NUMA domain (`--map-by numa --bind-to core`) means every rank gets its own local memory controller and L3 slice.

## CX3 reality check

Asking PBS for `ncpus=8` does not guarantee contiguous cores. A real run of ours landed threads on cores `12, 30, 31, 47, 61, 62, 67, 79` — scattered across both sockets. Pinning still helps within the cpuset PBS gave you, but for true locality request a whole NUMA domain (`ncpus=16`) or a whole socket (`ncpus=64`).

## Related

- [[Intel Ice Lake Architecture]] — the simpler 2-NUMA alternative tier.
- [[NUMA Latency]] — measured numbers.
- [[Topology Inspection]] — how to see this from inside a job.
- [[../openmp/Thread Pinning]] — controlling placement.
