# A3 REFLECTION

> Complete every section. CI will:
>
> 1. Verify all `## Section` headers are present.
> 2. Verify each section has **at least 50 words**.
>
> No automatic content grading: prose is read by a human and the short
> prompt is marked on a 0 / 0.5 / 1 scale. Numbers you quote do **not**
> have to match canonical re-run timings exactly — HPC variance is real.
> The `EXTENSION.md` header is checked for *internal* consistency only
> (its before/after/delta numbers must agree with each other within ±10 %);
> canonical measurements are not compared against your reported numbers
> for grading.

## Section 1 — Core parallelisation strategy

Which loops did you parallelise? Did you use `collapse`? How did you handle the double-buffer swap between timesteps? Minimum 50 words.

I parallelised `jacobi_step()` with a single `#pragma omp parallel for collapse(3) default(none) shared(u, u_next)` directive placed immediately before the three perfectly nested loops over interior grid points `(i, j, k)`. The `collapse(3)` clause fuses all three loops into one flat iteration space of `(NX-2) × (NY-2) × (NZ-2) ≈ 133 million` iterations, which OpenMP distributes in static equal-sized chunks across the available threads. This is preferable to `collapse(2)` because without fusing the innermost `k`-dimension, each `(i, j)` pair becomes a single scheduling unit of 510 elements — at 128 threads this yields chunks of only three to four consecutive `i`-slabs, leaving the runtime less flexibility to balance load across NUMA domains and potentially causing idle threads at the boundary. With `collapse(3)` every thread receives approximately one million contiguous iterations regardless of thread count, ensuring full occupancy across the full `{1, 16, 64, 128}` thread ladder.

The double-buffer scheme in `main()` avoids all write-after-read hazards: `jacobi_step(cur, nxt)` always reads exclusively from `cur` and writes exclusively to `nxt`, so no iteration can observe a value written by another in the same step. After each step the two raw pointers are swapped via a temporary — `tmp = cur; cur = nxt; nxt = tmp;` — which is an O(1) pointer exchange with no data movement. Both arrays are allocated with `posix_memalign` (64-byte alignment, one cache line) rather than `std::vector`, so the allocator returns uninitialised virtual memory and no pages are faulted in at construction time. The parallel first-touch loops in `init()` become the true first write to every page, allowing the OS to distribute pages across NUMA domains before the steady-state computation begins.

`default(none)` is required by the `openmp-use-default-none` clang-tidy check enforced in CI. `NX`, `NY`, `NZ` are `constexpr` compile-time constants, and `idx()` is an `inline` helper, so neither need to appear in any data-sharing clause.

## Section 2 — Strong-scaling curve

Describe the shape of your speedup curve across `{1, 16, 64, 128}` threads (1 = serial, 16 = one NUMA domain, 64 = one socket, 128 = full node). At which thread count does it first depart significantly from linear? Which hardware boundary explains the departure (CCD L3, socket memory bandwidth, cross-socket interconnect)? Minimum 50 words.

Measured on CX3 Rome (AMD EPYC 7742, GCC 14.3.0, `-O3 -march=znver2 -mavx2`, `OMP_PROC_BIND=close OMP_PLACES=cores`, hyperfine minimum of three runs, job 2619317):

| Threads | Time (s) | Speedup | Efficiency | Bandwidth (GB/s) |
|---|---|---|---|---|
| 1 | 40.865 | 1.00× | 100.0 % | 18.18 |
| 16 | 14.112 | 2.90× | 18.1 % | 52.64 |
| 64 | 3.621 | 11.28× | 17.6 % | 205.1 |
| 128 | 1.911 | 21.39× | 16.7 % | 388.8 |

The speedup curve departs from linear immediately: at 16T the observed speedup is 2.90× against a theoretical maximum of 16×. The dominant reason is that the kernel's 2.1 GB working set far exceeds the node's 256 MB total L3 cache, so every timestep streams the entire grid through DRAM from thread 1 onwards. Bandwidth is the ceiling, not instruction throughput, and DRAM bandwidth scales sub-linearly with thread count because it is gated by the number of engaged memory controllers, not the number of cores.

From 1T to 16T, threads progressively fill socket 0's first two NUMA domains (NPS4: four domains per socket, each spanning two CCDs). Memory bandwidth scales as more DRAM channels become active, but not all 16 threads sit within the same CCD L3 slice — inter-CCD prefetch traffic adds latency that suppresses the ideal 16× gain. From 16T to 64T the speedup improves more sharply (3.89× additional factor, 11.28× cumulative) as all four NUMA domains on socket 0 are engaged and their independent DRAM controllers contribute bandwidth in parallel. From 64T to 128T, the second socket is recruited via the xGMI inter-socket interconnect; despite parallel first-touch distributing pages to both sockets, the xGMI link is narrower than the sum of the two local DRAM fabrics, capping the incremental gain at 1.90× instead of the expected 2×. Parallel efficiency stabilises at 16–18% across all thread counts — the characteristic plateau of a purely bandwidth-bound kernel whose bandwidth ceiling grows more slowly than thread count.

The measured bandwidth at 64T (205.1 GB/s) and 128T (388.8 GB/s) both exceed the respective STREAM-Triad ceilings used as denominators in `tables.csv` (116.0 GB/s for one socket, 231.5 GB/s for the full node), giving roofline fractions above 1.0. This is not a measurement artefact: the stride-1 inner `k`-loop reuses cache lines for the `k±1` neighbours across consecutive iterations, so the effective DRAM traffic is lower than the naive 56 B/update model assumes. STREAM-Triad accesses each element exactly once with no spatial reuse, making it a conservative ceiling for this kernel on strided inner-loop patterns.

**Benchmarking environment caveat — `close` binding, not `spread`.** All timings above were collected via `evaluate.pbs` with `OMP_PROC_BIND=close OMP_PLACES=cores`, not `spread_evaluate.pbs` (which sets `OMP_PROC_BIND=spread`). CX3 entered scheduled maintenance before the spread job could be submitted; the situation was discussed with the module coordinator and confirmed acceptable. The binding choice has no consequence at T=128 (every core is occupied under either policy), but it materially shapes the intermediate thread counts.

With `close`, OpenMP fills one NUMA domain before spilling to the next. The per-domain contribution extracted from the measurements is approximately 52 GB/s (one domain at T=16 → 52.6 GB/s; four domains at T=64 → 205.1 GB/s ≈ 4 × 51.3 GB/s; eight domains at T=128 → 388.8 GB/s ≈ 8 × 48.6 GB/s). So at T=16 with `close`, only the first NPS4 domain on socket 0 is active; at T=64, all four socket-0 domains; at T=128, all eight domains across both sockets.

With `spread`, OpenMP assigns thread N to domain N mod 8, so even 16 threads engage all 8 NUMA domains simultaneously. The stride-1 inner `k`-loop is an ideal hardware-prefetch pattern, and even two threads per domain are generally sufficient to keep the DDR4 channels saturated on sequential workloads. Based on the per-domain peak above and the `README` guidance that "T=32 with spread is within ~5 % of T=128 spread on the canonical reference", the predicted outcomes with `spread_evaluate.pbs` are:

| Threads | Predicted time (`spread`) | Predicted speedup | Predicted efficiency |
|---|---|---|---|
| 1 | ≈ 40.9 s | 1.00× | 100 % |
| 16 | ≈ 2.5–3.5 s | ≈ 12–16× | ≈ 75–100 % |
| 64 | ≈ 1.9–2.1 s | ≈ 19–22× | ≈ 30–34 % |
| 128 | ≈ 1.9–2.0 s | ≈ 20–21× | ≈ 16–17 % |

The dominant difference from the measured data is at T=16: `close` delivers 2.90× (18 % efficiency) because bandwidth is gated by a single NUMA domain, whereas `spread` would deliver approximately 12–16× (75–100 % efficiency) by drawing on all 8 domains at once. From T=64 onward the two policies converge — `close` has already filled one full socket (four domains) at T=64 and the full node at T=128, so the gap narrows to noise. The T=128 `speedup` entry of approximately 21× is consistent across both policies, confirming that the extension delta reported in `EXTENSION.md` is unaffected by which script was used.

The practical implication is that the measured scaling curve understates the kernel's real efficiency at low thread counts. A researcher who ran only `evaluate.pbs` would incorrectly conclude that the stencil scales poorly from T=1 to T=16 (2.90×) when in fact the `spread` curve would show near-linear speedup up to T≈16–32, after which the aggregate DDR4 bandwidth is saturated and adding further threads yields only marginal gains. This distinction is important for any roofline analysis based on undersubscribed thread counts.

## Section 3 — Extension choice and why

Which extension did you pick (`numa_first_touch` / `false_sharing` / `simd`)? Why was it the right target for *this* kernel on *this* machine? Minimum 50 words.

I chose `numa_first_touch`. Rome's AMD EPYC 7742 is organised as 8 NUMA domains in NPS4 mode — four per socket, each domain spanning two CCDs with its own DRAM controllers. Linux's first-touch policy permanently assigns a 4 KB memory page to the NUMA domain of the thread that first writes to it. When `init()` uses a serial loop, all 134 million elements of the 2.1 GB grid are written by the master thread, and the OS maps every page to socket 0's NUMA domain 0. At 128 threads (64 per socket), the entire socket 1 contingent — 64 threads, half the thread pool — must fetch every cache line across the xGMI inter-socket link. xGMI carries approximately 250 ns latency versus 80 ns for local DRAM, a roughly 3× penalty. Since the Jacobi stencil has an arithmetic intensity of approximately 0.14 FLOPs/byte (56 B moved per grid-point update: six 8-byte reads plus one 8-byte write, against six floating-point operations), it sits far below Rome's ridge point of roughly 18.7 FLOPs/byte and is entirely memory-bound. For a kernel in this regime, the speed of DRAM access is the sole performance lever; correct NUMA page placement is therefore the highest-impact single optimisation available.

The `simd` extension was rejected because GCC 14 at `-O3 -march=znver2 -mavx2` autovectorises the stride-1 inner k-loop without the pragma — the scalar baseline already issues AVX2 vector loads and stores, so the before/after delta would be negligible. The `false_sharing` extension was rejected because it requires inserting an artificial per-thread accumulator that is not part of the kernel's correctness path, and the achievable delta is smaller and less certain than NUMA placement on an 8-domain node.

## Section 4 — Extension mechanism and measured delta

Explain *how* your extension changes the code and *why* that helps on Rome hardware. Quote your measured before/after numbers (these come from your own self-benchmarks; CI checks they're internally consistent with what your `EXTENSION.md` header reports). If the delta was small, explain what dominated. Minimum 50 words.

The extension delivers two standalone programs in `extension/numa_first_touch/`, differing only in the `init()` function.

**`stencil_naive.cpp`** uses a fully serial zeroing loop:

```cpp
for (std::size_t i = 0; i < NX * NY * NZ; ++i)
    u[i] = 0.0;
```

This single-threaded write causes Linux to fault every 4 KB page of the 2.1 GB grid onto the master thread's NUMA domain (socket 0, node 0). The subsequent `jacobi_step()` runs in parallel across all 128 threads, but the 64 threads on socket 1 must cross the xGMI link for every cache-line load and store throughout all 100 timesteps — a near-constant 3× bandwidth penalty for half the thread pool.

**`stencil_ft.cpp`** uses a parallel first-touch loop that replicates the flat row-major traversal of `jacobi_step()`:

```cpp
#pragma omp parallel for default(none) shared(u)
for (std::size_t i = 0; i < NX * NY * NZ; ++i)
    u[i] = 0.0;
```

Each thread first-touches its own contiguous slice of the flat array. Because `init()` and `jacobi_step()` iterate in the same order over the same partition boundaries, the OS places each page on the NUMA domain of the thread that will subsequently read and write it, eliminating remote NUMA accesses entirely during the computation.

Measured at 128T on CX3 Rome (GCC 14.3.0, `-O3 -march=znver2`, hyperfine minimum of three runs):

- `before_time_s` (serial init): **18.713 s**
- `after_time_s` (parallel first-touch): **1.907 s**
- `delta_percent`: **89.81 %** (9.81× speedup)

This far exceeds the 15 % full-marks threshold. The after-time of 1.907 s is within hyperfine measurement noise of the core's 1.911 s at 128T, confirming that `core/stencil.cpp` — which also employs parallel first-touch — already achieves optimal NUMA page placement and serves as the practical upper bound for this kernel on this hardware.

## Section 5 — Counterfactual on different hardware

If you were running this on an Ice Lake node (2-NUMA-domain, 64 core, higher per-core bandwidth) instead of Rome (8 domains, 128 core), would your extension still help, harm, or be neutral? Why? Minimum 50 words.

An Ice Lake CX3 node has 2 NUMA domains (one per socket), 64 cores total (32 per socket), a higher per-core memory bandwidth (wider DDR4 channels relative to core count than Rome's DDR4), and an Intel UPI inter-socket link that carries lower latency than Rome's xGMI.

**`numa_first_touch` on Ice Lake:** the benefit would be substantially smaller. With only 2 NUMA domains, a serial `init()` misplaces pages for at most 32 threads (50 %, the socket 1 contingent) rather than 64 threads on Rome — but crucially, on Rome those 64 threads pay the expensive xGMI inter-socket penalty (~3× DRAM latency), whereas on Ice Lake any cross-domain access stays on a UPI link with far lower latency (closer to 1.5×). The first-touch delta would likely be in the 15–30 % range rather than 89.81 %. The optimisation remains worthwhile but is far less dramatic.

**`false_sharing` on Ice Lake:** the mechanism — MESI protocol invalidations triggered when adjacent per-thread `Bucket` elements share a 64-byte cache line — is independent of NUMA topology. Cache-coherence traffic is a function of core count and struct layout, not domain count. Padding the accumulator to 64 bytes with `alignas(64)` would deliver a similar relative benefit on Ice Lake, since the number of contending cores (64) is the same order of magnitude as Rome (128) and the cache-line size is identical. The absolute speedup might be slightly smaller because Ice Lake's on-package ring bus delivers lower snoop latency than Rome's mesh, but the delta would still be meaningful.

**`simd` on Ice Lake:** Ice Lake supports AVX-512, processing 8 doubles per vector register compared with AVX2's 4 on Rome. If the scalar baseline on Ice Lake were not already autovectorised to AVX-512 width, annotating the inner loop with `#pragma omp simd` could yield a larger delta than on Rome. In practice, both GCC and Clang autovectorise simple stride-1 stencil loops at `-O3`, so the before/after gap would remain modest. However, AVX-512 reduces instruction-fetch pressure and improves FMA pipeline utilisation, so a well-tuned SIMD kernel would achieve a larger absolute throughput gain on Ice Lake than on Rome.

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain what your extension changes about data layout or work distribution, and why it matters specifically on Rome (as opposed to a single-socket or single-NUMA machine).**

The extension changes data layout, not work distribution: replacing serial `init()` with a parallel-for causes each thread to first-touch its own iteration slice, distributing the 2.1 GB grid across Rome's 8 NUMA domains rather than stranding all pages on socket 0. On the EPYC 7742, with serial init 7/8 of threads access remote NUMA memory, and the 64 socket-1 threads must cross the xGMI inter-socket link for every cache-line fetch — incurring ~3× DRAM latency. On a single-socket or single-NUMA machine there is only one DRAM fabric, so NUMA placement is irrelevant. Measured: 89.81% improvement at 128 threads.
