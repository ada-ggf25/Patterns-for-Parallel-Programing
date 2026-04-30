# A3 — MCQ Answers and Rationale

All 15 questions from `questions.md` with correct answers and brief reasoning. Fill `answers.csv` from this table.

## Quick-reference answer key

| qid | Answer |
|---|---|
| q01 | C |
| q02 | A |
| q03 | B |
| q04 | B |
| q05 | A |
| q06 | A |
| q07 | B |
| q08 | B |
| q09 | C |
| q10 | A |
| q11 | A |
| q12 | A |
| q13 | B |
| q14 | A |
| q15 | C |

---

## Detailed rationale

### q01 — Operational intensity of the 7-point Jacobi stencil

**Answer: C — 0.14 (strongly memory-bound)**

The **actual A3 kernel** reads 6 face-neighbour doubles and writes 1 per grid point: **56 bytes** (6×8 + 1×8) for **6 FLOPs** (5 adds + 1 divide) → naive OI = 6/56 ≈ **0.11 FLOPs/byte**; with cache-line reuse factored in ≈ **0.14 FLOPs/byte**. The **pedagogical slide model** uses 7 reads including the centre (64 bytes, 7 FLOPs → 7/64 ≈ 0.11); both give the same OI range, but the A3 code excludes the centre. Rome's ridge point is ~18.7 FLOPs/byte, so this kernel is ~130× below the ridge — firmly in the bandwidth-bound region. The working set (2.1 GB) exceeds Rome's L3 (256 MB per socket), so data comes from DRAM.

### q02 — What "first-touch" means on a NUMA system

**Answer: A — page allocated on NUMA node where the thread that first writes runs**

The OS allocates a virtual page only when a thread first accesses it. "First touch" specifically means the physical page is placed on the NUMA node of the writing thread. This is the default Linux NUMA policy (`MPOL_LOCAL`). The other options are wrong: first-touch is not a lock (B), not forced to node 0 (C — that would be `--membind=0`), and not prefetching (D).

### q03 — How to avoid NUMA penalties in 3D Jacobi

**Answer: B — `#pragma omp parallel for` loop using the same traversal order as the compute step**

With serial init (A), all pages land on the master thread's NUMA node — 7/8 threads are on remote NUMA domains at 128T; of those, 4/8 cross the socket boundary via xGMI (~3× penalty). `calloc` (C) is also single-threaded from the allocator's NUMA perspective. `mmap` (D) likewise allocates pages lazily but doesn't distribute them. Only a parallel init with matching traversal order (B) places each page near the thread that will compute it.

### q04 — Definition of false sharing

**Answer: B — two threads write to different addresses that happen to land on the same cache line**

False sharing: logically-independent data sits in the same 64-byte cache line. Every write by one thread forces all other threads to invalidate their copy of that line (MESI state → Invalid), causing unnecessary cache misses. A (same address) is a true data race. C (read before write) is a race/ordering issue. D (compiler reordering) is a separate memory model concern.

### q05 — Thread placement with `OMP_PLACES=cores OMP_PROC_BIND=close`

**Answer: A — threads bound to specific cores; consecutive thread IDs land on nearby cores**

`OMP_PROC_BIND=close` means each thread in the team is placed as close as possible to the master thread's place. Combined with `OMP_PLACES=cores`, consecutive thread IDs map to physically adjacent cores (same CCD first, then same socket). They do NOT migrate freely (B), they are NOT all on socket 0 (C) — they spread across both sockets at 128T, and hyperthreads are not auto-enabled (D) unless places includes SMT threads.

### q06 — Roofline fraction: 35 GB/s achieved vs 230 GB/s STREAM

**Answer: A — 0.15 (substantial headroom)**

Roofline fraction = achieved BW / STREAM BW = 35 / 230 ≈ 0.152. This falls in the ≥ 0.15 but < 0.30 band → earns 0.9/6 pts for the roofline component. "Substantial headroom" is accurate: this kernel is getting only 15% of the available memory bandwidth, leaving 85% unused.

### q07 — Implicit sync at end of `#pragma omp for`

**Answer: B — implicit barrier**

By default, every `#pragma omp for` (and `#pragma omp single`) has an **implicit barrier** at the end: all threads in the team wait until every thread has finished its share of the loop before any thread proceeds. This is how the OpenMP memory model ensures writes from the loop are visible to the next construct. Use `nowait` to suppress it. `critical`, `atomic`, and `flush` (A/C/D) are separate synchronisation constructs, not implicit.

### q08 — Zero speedup at 64 threads despite correct first-touch

**Answer: B — bandwidth at one socket is saturated**

If first-touch is correct, NUMA placement is good — rule out cross-socket penalty. At 64 threads (one full socket), the single-socket STREAM bandwidth (~116 GB/s) is already saturated. The Jacobi kernel is fully bandwidth-bound, so adding more threads within the same socket cannot increase bandwidth — you need more sockets (going to 128T crosses to the second socket). This explains a plateau at 64T that breaks at 128T.

### q09 — Padding stride for false-sharing mitigation

**Answer: C — 64 bytes (one cache line on Rome / most x86)**

Cache lines on all modern x86 CPUs (including Rome / Zen 2) are exactly 64 bytes. To ensure two adjacent `Bucket` structs cannot share a cache line, the stride between them must be ≥ 64 bytes. `sizeof(double) = 8` (A) is 8× too small. 32 bytes (B) is still half a cache line. 4096 bytes (D) would work but wastes memory.

```cpp
struct alignas(64) Bucket { double v; char pad[56]; };  // 64 bytes total
```

### q10 — `#pragma omp simd safelen(8)` semantics

**Answer: A — at least 8 consecutive iterations are safe to execute concurrently without cross-iteration data deps**

`safelen(N)` is a programmer assertion: iterations `i` and `i+N` (or any two iterations ≥ N apart) have no data dependence. This lets the compiler generate vector code for chunks of N. It does NOT force 8-wide instructions (B — that's hardware-dependent), does NOT spawn threads (C), and does NOT count operations (D).

### q11 — Why `#pragma omp simd` helps on Rome's innermost Jacobi loop

**Answer: A — AVX2 lets the CPU process multiple grid points per instruction**

Zen 2 has AVX2 256-bit SIMD registers: 4 doubles fit in one register. `#pragma omp simd` tells the compiler the k-loop has no cross-iteration dependences, enabling 4-wide vectorised load/add/store. This improves compute throughput per cycle. It does NOT reduce memory bandwidth (B — same data must still move), does NOT disable threading (C), and does NOT relate to NUMA placement (D).

### q12 — Compute `delta_percent` for before=4.8s, after=3.2s

**Answer: A — 33.3%**

```
delta_percent = (before - after) / before × 100
             = (4.8 - 3.2) / 4.8 × 100
             = 1.6 / 4.8 × 100
             = 33.33 %
```

B (50%) would be `(4.8 - 3.2) / 3.2 × 100` (wrong denominator). C (1.5) is the raw ratio minus 1, not percentage. D is negative (wrong direction — "after" is faster, not slower).

### q13 — Bandwidth-limited GFLOPs ceiling: OI=0.14, STREAM=230 GB/s

**Answer: B — 32 GFLOPs/s**

Bandwidth-limited ceiling = OI × STREAM bandwidth = 0.14 FLOPs/byte × 230 GB/s = 32.2 GFLOPs/s. This is the roofline ceiling for a memory-bound kernel: no matter how many FLOPs the hardware can theoretically do, the rate is capped by how fast data arrives from DRAM.

### q14 — Preventing reads of in-progress writes between Jacobi timesteps

**Answer: A — double-buffering: read from grid A, write to grid B, swap pointers between steps**

Double-buffering is the standard technique: maintain two grids `a` and `b`. One step reads `a` and writes `b`; the next reads `b` and writes `a`. `std::swap(a, b)` between timesteps swaps pointers in O(1). This guarantees you never read a partially-updated grid. `atomic` on every update (B) and `critical` around the whole step (C) would both be catastrophically slow and don't solve the read/write ordering issue. `flush` (D) is not sufficient to prevent mid-step reads.

### q15 — SIMD extension reports only 1.02× speedup

**Answer: C — report the measured 2% delta; analyse in REFLECTION.md why vectorisation didn't help more**

Honest reporting is required. Don't fabricate numbers (A). Don't leave `delta_percent` blank (B — CI checks consistency). Don't switch extensions (D — that's not allowed after declaring). Instead, report the real 1.02× ratio and write a technical analysis: the baseline Clang-18 likely auto-vectorised the scalar variant already, and the kernel is memory-bound (bandwidth, not compute, is the bottleneck) so vectorisation buys little extra throughput.

---

## Related

- [[A3 Progress]] — phase-by-phase implementation guide; Phase 5 is the MCQ phase.
- [[A3 Jacobi]] — technical reference for kernel and extension details.
- [[../performance/NUMA First Touch]] — q02, q03, q05, q08 context.
- [[../performance/False Sharing]] — q04, q09 context.
- [[../performance/SIMD]] — q10, q11, q15 context.
- [[../performance/Roofline Model]] — q01, q06, q13 context.
- [[../performance/Loop Transformations]] — collapse(3) and double-buffering.
- [[../openmp/Barriers]] — q07 (implicit barrier at end of `omp for`).
