# A1 Benchmark Results — CX3 Rome

> Measured 2026-04-29. Platform: CX3 Rome (AMD EPYC 7742, 128 cores, 2 sockets).
> Build: GCC 13.3.0, `-O3 -march=znver2 -mavx2`. Harness: `hyperfine --warmup 1 --min-runs 3`.
> All times are the `min` value from hyperfine JSON. N = 100,000,000.

## Summary — Winner at 128 threads

**`schedule(guided)` wins at 128T with T(128) = 0.0446 s, speedup = 42.49×.**

| Schedule | T(1) s | T(16) s | T(64) s | T(128) s | Speedup@128 | Eff@128 |
|---|---|---|---|---|---|---|
| `static` | 1.8939 | 0.3376 | 0.0961 | 0.0566 | 33.46× | 0.261 |
| `dynamic,64` | 1.9048 | 0.1401 | 0.0697 | **0.0987** | 19.31× | 0.151 |
| `guided` | 1.8961 | 0.2297 | 0.0719 | **0.0446** | 42.49× | 0.332 |

### Key observations

1. **`guided` is the overall winner** at 128T, beating static by 21 % and dynamic(64) by 2.2×.
2. **`dynamic,64` regresses from 64T → 128T** (0.0697 s → 0.0987 s). Chunk=64 is too fine: N=1e8 ÷ 64 ≈ 1.5 M chunks, so 128 threads each make ~12 k atomic fetch-and-add dispatches — dispatcher contention dominates compute. `guided` avoids this by starting with large chunks and tapering, keeping dispatch count low.
3. **`static` is best at T(1)** (single thread; all schedules should tie but `static` has the lowest loop overhead with no dynamic dispatch at all).
4. **`dynamic,64` wins at T(16)** (0.1401 s vs guided 0.2297 s), because 16 threads can handle chunk=64 without saturating the dispatcher, and fine-grained balancing distributes the spike region optimally. At 64T it's also marginally better than `guided` (0.0697 vs 0.0719 s).

### Schedule to hardcode in `integrate.cpp`

```cpp
schedule(guided)
```

Replace `schedule(runtime)` with `schedule(guided)` before final submission. `runtime` silently falls back to `static` when `OMP_SCHEDULE` is unset (the grader environment), which gives 0.0566 s instead of 0.0446 s — a ~21 % performance loss.

---

## Full data for `guided` (submitted in `tables.csv`)

| thread_count | measured_time_s | measured_speedup | measured_efficiency |
|---|---|---|---|
| 1 | 1.896130 | 1.00 | 1.00 |
| 16 | 0.229710 | 8.2544 | 0.5159 |
| 64 | 0.071898 | 26.3725 | 0.4121 |
| 128 | 0.044624 | 42.4911 | 0.3320 |

### Scaling analysis

- **1→16T:** speedup 8.25× out of linear 16× → efficiency 51.6 %. The spike region (x ∈ 0.3–0.4, ~10 M iters) is distributed, but guided starts with large chunks so the first few assignments carry residual imbalance.
- **16→64T:** efficiency drops from 51.6 % to 41.2 % — NUMA effects begin (64 cores span both sockets on a 2×64-core Rome).
- **64→128T:** efficiency drops to 33.2 % — cross-socket NUMA latency, bandwidth sharing, fork-join overhead, and reduction tree (log₂128 = 7 steps) all compound.

---

## Roofline position (at 128T, guided)

```
estimated FLOPs/iter = 0.9 × 5 + 0.1 × 35 = 8 FLOPs
total FLOPs          = 8 × 1e8 = 0.8 GFLOPs
achieved GFLOPs      = 0.8 / 0.044624 = 17.93 GFLOPs
vs 4608 GFLOPs peak  = 0.39 %
vs 2896 GFLOPs HPL   = 0.62 %
```

The kernel is **compute-bound** (OI >> ridge ≈ 18.7 FLOPs/byte) but achieves only ~0.4 % of theoretical peak because:
- Scalar execution only (no SIMD — spike branch defeats auto-vectorisation).
- 1 FLOPs/cycle vs 16 FLOPs/cycle for AVX2 FMA.
- Transcendental functions (`sin`, `cos`, `sqrt`) are not pipelined.

---

## Raw JSON min times

| Schedule | T(1) | T(16) | T(64) | T(128) |
|---|---|---|---|---|
| `static` | 1.893903 | 0.337551 | 0.096057 | 0.056605 |
| `dynamic,64` | 1.904769 | 0.140149 | 0.069656 | 0.098660 |
| `guided` | 1.896130 | 0.229710 | 0.071898 | 0.044624 |

Source files: `bench/static/`, `bench/dynamic_64/`, `bench/guided/` — each contains `integrate-{1,16,64,128}.json`.

---

## Related

- [[A1 Progress]] — checklist and current status.
- [[A1 Integration]] — technical reference: kernel, schedules, REFLECTION guide.
- [[../openmp/Schedules]] — guided vs dynamic chunk-size theory.
- [[../performance/Performance Metrics]] — speedup/efficiency formulas.
- [[../performance/Roofline Model]] — OI, Rome ceiling constants.
- [[../performance/Six Sources of Overhead]] — why efficiency drops at high thread counts.
