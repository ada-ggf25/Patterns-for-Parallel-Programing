# A1 REFLECTION

> Complete every section. CI will:
>
> 1. Verify all `## Section` headers below are present.
> 2. Verify each section has **at least 50 words**.
>
> No automatic content grading: the prose is read by a human, and the short
> prompt at the end is marked on a 0 / 0.5 / 1 scale. The numbers you quote
> in your reflection do **not** have to match canonical times exactly — HPC
> queue variance is real. Be concise, ground claims in your measurements, show your working.

## Section 1 — Schedule choice and why

Which schedule (`static` / `dynamic` / `guided` / chunk size) did you end up with, and why? Reference the cost structure of `f(x)` and what the measured timings told you. Mention at least one schedule you tried and discarded, and what the measured evidence was. Minimum 50 words.

I chose `schedule(guided)`. The integrand `f(x)` has a spike region for x ∈ (0.3, 0.4) where per-call cost is approximately 10× higher than the rest of the domain due to an inner loop executing ten iterations of `abs`, `sqrt`, and addition. This non-uniform cost makes schedule choice critical at high thread counts.

I measured all three schedules on a 128-core CX3 Rome node. With `static`, spike iterations land on a fixed thread range, leaving the remaining threads idle at the barrier: T(128) = 0.0566 s. I tried `dynamic,64` next — it distributes the spike well at 16 threads (T(16) = 0.1401 s) but regresses at 128 threads (T(128) = 0.0987 s, slower than at 64 threads). Chunk size 64 generates approximately 1.5 million atomic dispatches at 128 threads, saturating the work queue. `guided` starts with large chunks that taper toward the loop end, keeping dispatch count low while still distributing the spike across all threads. It achieved T(128) = 0.0446 s — 21% faster than static and 2.2× faster than dynamic,64.

## Section 2 — Scaling behaviour

Looking at your `tables.csv`, where does your speedup curve depart from ideal (linear)? What does that tell you about overhead, memory bandwidth, or load balance for this kernel? Minimum 50 words.

Speedup departs from linear immediately. At 16 threads the measured speedup is 8.25× against a linear ideal of 16×, giving efficiency 0.5159. At 64 threads, speedup is 26.37× (efficiency 0.4121), and at 128 threads, 42.49× (efficiency 0.3320). The curve flattens progressively rather than hitting a sharp wall.

Several overhead sources compound at high thread counts. The Rome node has two 64-core sockets: beyond 64 threads, work crosses a NUMA boundary, increasing memory latency for reduction-tree operations. The guided schedule's tapering dispatch also introduces some serial contention as chunk sizes shrink near the loop end. Fork-join overhead and the implicit barrier become a larger fraction of total runtime as T(P) shrinks. The final reduction requires log₂(128) = 7 combine steps. Amdahl's law sets a hard ceiling: the serial endpoint computation `0.5*(f(a)+f(b))` and output are not parallelised, and their fixed cost limits the maximum achievable speedup regardless of thread count.

## Section 3 — Roofline position

Pick your best thread count. Using the Rome roofline constants from the day-2 slides (theoretical peak 4608 GFLOPs, HPL-achievable 2896 GFLOPs, STREAM triad 246 GB/s), what roofline fraction did you achieve against the *theoretical* and the *HPL-achievable* compute ceilings? Most non-DGEMM code (including A1) lands well below both — explain why your kernel doesn't approach DGEMM-class efficiency. If you want to argue your kernel is bandwidth-bound rather than compute-bound, justify it. Minimum 50 words.

The kernel is compute-bound. Each call to `f(x)` reads only the input `x` (already in a register) and returns a scalar — memory traffic per iteration is negligible, placing operational intensity far above the ridge point of 4608 / 246 ≈ 18.7 FLOPs/byte. The base case costs approximately 5 FLOPs (`sin`, `cos`, one multiply, one add); the spike case costs approximately 35 FLOPs (same base plus ten iterations of `abs`, `sqrt`, and add). Weighted average: 0.9 × 5 + 0.1 × 35 = 8 FLOPs/iteration.

At 128 threads with T(128) = 0.0446 s and N = 10⁸ iterations, total FLOPs ≈ 0.8 GFLOPs, giving achieved throughput of 0.8 / 0.0446 ≈ 17.93 GFLOPs. Against the theoretical peak of 4608 GFLOPs this is 0.39%; against the HPL-achievable ceiling of 2896 GFLOPs it is 0.62%.

The fraction is low because the kernel runs in scalar mode. The conditional branch in `f(x)` for the spike region prevents the compiler from emitting AVX2 FMA instructions — divergent control flow breaks SIMD. Without vectorisation, each core delivers approximately 1 FLOP/cycle rather than the 16 FLOPs/cycle available with double-precision AVX2 FMA. Transcendental functions (`sin`, `cos`, `sqrt`) are also not pipelined. These factors together explain why a compute-bound kernel still achieves under 1% of theoretical peak.

## Section 4 — What you'd try next

You have two more days. What would you change about `integrate.cpp`? Pick one concrete change and predict its effect. Minimum 50 words.

I would restructure `f(x)` to eliminate the spike branch and then apply `#pragma omp simd` to the inner loop. The current `if (x > 0.3 && x < 0.4)` branch prevents SIMD because the compiler cannot guarantee uniform control flow across a vector lane group. The fix is to compute both code paths unconditionally and select the result with a mask: `result = base_result + mask * (spike_extra)`, where `mask = (x > 0.3 && x < 0.4) ? 1.0 : 0.0`. With divergence eliminated, `#pragma omp simd` on the `for` loop would allow the compiler to pack four double-precision values into a single AVX2 register and execute four iterations per instruction. The base-case throughput would increase approximately 4×, from ~1 FLOP/cycle to ~4 FLOPs/cycle. Combined with the existing 128-thread parallelism, achieved GFLOPs could rise from ~18 to ~70, pushing the HPL roofline fraction from 0.62% to roughly 2.4%.

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain why your chosen schedule is appropriate for the cost structure of this particular `f(x)`.**

`f(x)` has a spike region for x ∈ (0.3, 0.4) where per-call cost is ~10× higher than elsewhere. `static` clustering all spike iterations on one thread range gave T(128) = 0.0566 s. `dynamic,64` distributes the spike but generates ~1.5 million atomic dispatches at 128 threads, causing queue contention and regression to T(128) = 0.0987 s. `guided` starts with large chunks — reducing total dispatch count — and tapers chunk size to rebalance the remaining work, achieving T(128) = 0.0446 s. The decreasing-chunk strategy amortises dispatch overhead while still spreading the spike across all 128 threads.
