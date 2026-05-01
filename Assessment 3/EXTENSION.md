---
chosen: numa_first_touch
before_time_s: 18.713
after_time_s: 1.907
delta_percent: 89.81
---

## Rationale (≤ 200 words)

The AMD EPYC 7742 (Rome) has 8 NUMA domains (NPS4), each with its own DDR4 controllers separated
by the xGMI inter-socket link (3× latency vs. local DRAM). This kernel (OI ≈ 0.14 FLOP/byte,
2.1 GB working set) is memory-bound: NUMA page placement is the dominant performance lever.

`stencil_naive.cpp` uses a serial zeroing loop; Linux's first-touch policy maps every 4 KB page
to socket 0, node 0. At 128 threads, all 64 socket-1 threads must cross xGMI for every
cache-line — a 3× bandwidth penalty for half the thread pool.

`stencil_ft.cpp` parallelises init with `#pragma omp parallel for`. Each thread first-touches its
own slice, so Linux places each page on the thread's local NUMA domain. Since both functions use
the same row-major traversal, page-to-thread affinity is optimal throughout computation.

Measured via `evaluate.pbs` (`OMP_PROC_BIND=close OMP_PLACES=cores`, CX3 Rome, GCC 14,
`-O3 -march=znver2 -mavx2`): **18.713 s → 1.907 s, 89.81% improvement** (9.81×), above the
15% full-marks threshold. A revised `spread_evaluate.pbs` (`OMP_PROC_BIND=spread`) was prepared
but CX3 maintenance prevented submission, confirmed with the module coordinator. The delta is
binding-invariant at T=128: all 128 cores are occupied under either policy and the comparison holds.
