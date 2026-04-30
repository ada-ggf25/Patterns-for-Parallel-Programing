# A3 — 3D Jacobi Stencil (40 pts)

Parallelise a bandwidth-bound 7-point 3D Jacobi stencil and then apply one of three hardware-level optimisations as an extension.

## Kernel (A3-core)

7-point stencil over a 3D grid for `NSTEPS` timesteps:

```cpp
u_next[i,j,k] = (u[i-1,j,k] + u[i+1,j,k]
               + u[i,j-1,k] + u[i,j+1,k]
               + u[i,j,k-1] + u[i,j,k+1]) / 6.0;
```

> **Note:** The stencil is called "7-point" because it spans 7 grid points (centre + 6 face neighbours), but the update formula uses only the **6 face neighbours** divided by 6 — the centre is excluded. Do not include `u[i,j,k]` and do not divide by 7.

**OI ≈ 0.14 FLOPs/byte → bandwidth-bound, well below Rome's ridge at 18.7.**

The canonical parallelisation:

```cpp
#pragma omp parallel for collapse(3) default(none) shared(u, u_next)
for (size_t i = 1; i < NX-1; ++i)
    for (size_t j = 1; j < NY-1; ++j)
        for (size_t k = 1; k < NZ-1; ++k)
            u_next[idx(i,j,k)] = (u[idx(i-1,j,k)] + u[idx(i+1,j,k)] +
                                   u[idx(i,j-1,k)] + u[idx(i,j+1,k)] +
                                   u[idx(i,j,k-1)] + u[idx(i,j,k+1)]) / 6.0;
```

`collapse(3)` distributes all `(NX-2)×(NY-2)×(NZ-2)` iterations — essential when any single dimension is small.

> `NX`, `NY`, `NZ` are `constexpr` in `stencil.h` — they are compile-time constants, not runtime variables. They do **not** need to be listed in `shared` or `firstprivate`. `idx` is `inline` in `stencil.cpp`. CI enforces `default(none)` via the `openmp-use-default-none` clang-tidy check.

## NUMA-aware initialisation

**Critical for A3-core performance:** initialise `u` in the same parallel-for pattern you'll use for computation:

```cpp
// BAD — all pages on master's NUMA node → 7/8 cross-socket at 128 threads
for (size_t i = 0; i < NX*NY*NZ; ++i) u[i] = 0.0;

// GOOD — each thread first-touches its own pages
#pragma omp parallel for default(none) shared(u)
for (size_t i = 0; i < NX*NY*NZ; ++i) u[i] = 0.0;
```

## Starter code structure (`core/stencil.cpp`)

The starter is a complete, compilable file. Functions and their status:

| Function | Status | Key note |
|---|---|---|
| `idx(i, j, k)` | Ready — do not change | Inline linear-address helper; defined in `stencil.cpp`, **not** `stencil.h` — each extension `.cpp` must define its own (or copy it from core) |
| `jacobi_step(u, u_next)` | **Your work** — serial with `// TODO` | Add `#pragma omp parallel for collapse(3) default(none) shared(u, u_next)` before the outermost loop |
| `checksum(u)` | Ready — do not change | Serial summation over all grid points; produces the reference output |
| `init(u)` | Ready — do not change | **Already parallel first-touch** via `#pragma omp parallel for default(none) shared(u)`; also sets Dirichlet BC on face `i=0` in a second parallel loop |
| `main()` | Ready — do not change | Allocates `a` and `b` as `std::vector<double>(NX*NY*NZ)`; calls `init(a)`; copies `a → b` with `memcpy`; runs `NSTEPS` iterations of `jacobi_step(a,b) + std::swap(a,b)` |

**Expected output** (verify your implementation produces this):

```text
checksum = 1.319003e+06
```

**Extension files are standalone programs.** Each `.cpp` placed in `extension/<branch>/` must contain its own `main()`. The simplest approach is to copy `core/stencil.cpp` in full (which brings `idx`, `checksum`, `init`, and `main`) and then modify only the function relevant to the chosen extension.

CMake auto-discovers `.cpp` files in extension directories and names the targets `ext_<branch>_<stem>`, e.g. `ext_numa_first_touch_stencil_naive`.

## Scoring: A3-core (25 pts)

| Component | Pts |
|---|---|
| Build + TSan clean | 2 |
| Correctness at `{1, 16, 64, 128}` | 8 (2 pts each) |
| **Roofline fraction** at 128T vs measured STREAM | 6 (graduated) |
| `tables.csv` internal consistency | 2 |
| Style | 2 |
| MCQ (15 questions) | 2 |
| REFLECTION (core sections) format + completion | 1 |
| Reasoning question (instructor-marked) | 2 |

Roofline scoring: ≥ 0.70 → 6; ≥ 0.50 → 4.2; ≥ 0.30 → 2.4; ≥ 0.15 → 0.9; else 0.

## Extension — pick ONE (15 pts)

### Option A: `numa_first_touch`

Swap serial init for parallel-init; measure `before` vs `after` timings.

```cpp
// before (stencil_naive.cpp — serial init, all pages on master's NUMA node)
for (std::size_t i = 0; i < NX * NY * NZ; ++i) u[i] = 0.0;

// after (stencil_ft.cpp — parallel first-touch, pages near their threads)
#pragma omp parallel for default(none) shared(u)
for (std::size_t i = 0; i < NX * NY * NZ; ++i) u[i] = 0.0;
```

> `NX * NY * NZ` are `constexpr` values — no runtime variable `n` needed. Use `shared(u)` only with `default(none)`.

Threshold: `delta_percent ≥ 15` → full; `≥ 5` → half.

### Option B: `false_sharing`

Find a per-thread accumulator with packed layout; pad it to cache-line boundary.

```cpp
// before
struct Bucket { double v; };

// after
struct alignas(64) Bucket { double v; char pad[56]; };
```

Threshold: `delta_percent ≥ 15` → full; `≥ 5` → half.

### Option C: `simd`

Annotate the innermost stencil loop with `#pragma omp simd`.

```cpp
// after
#pragma omp simd
for (size_t k = 1; k < NZ-1; ++k)
    u_next[idx(i,j,k)] = stencil(u, i, j, k);
```

Plain `#pragma omp simd` is sufficient — on Zen 2 (Rome) unaligned vector loads cost the same as aligned, so `aligned` and `safelen` are not required.

> **Auto-vectorisation caveat:** `evaluate.pbs` on CX3 builds with GCC 13 and `-O3 -march=znver2 -mavx2`. At `-O3` GCC already autovectorises simple stencils without the pragma, so the `scalar` baseline may already be fully vectorised. Expect a small delta (< 5%). CI uses Clang-18 at `-O2`, which is less aggressive. Document this in REFLECTION.md Section 4.

Threshold: `ratio ≥ 1.2×` → full; `≥ 1.05×` → half.

## Scoring: A3-extension (15 pts)

| Component | Pts |
|---|---|
| Implementation (before + after build, run, correct) | 7 |
| Soft-threshold delta | 5 |
| Reasoning question (instructor-marked) | 3 |

## `EXTENSION.md` format

```yaml
---
chosen: numa_first_touch
before_time_s: 4.52
after_time_s:  2.81
delta_percent: 37.8
---

## Rationale (≤ 200 words)
<your prose>
```

`delta_percent = (before - after) / before × 100`. Grader checks this is consistent with your reported times within ±10 %.

> **Section heading must be `## Rationale`** — this is the exact heading in the starter `EXTENSION.md`. The CI cross-checks `before_time_s`, `after_time_s`, and `delta_percent` for internal consistency, not the prose heading, but use the exact starter heading to avoid any surprises.

## Diagnostic decision tree

| Symptom | Likely cause |
|---|---|
| 1.0× speedup at any P | Pragma typo, serial init, code actually serial |
| 4× at P=16, then plateau | Serial init → all pages on one NUMA domain |
| 8× at P=64, no gain at P=128 | Single socket bandwidth saturated |
| Large run-to-run variance | No warm-up or cold cache |

## `tables.csv` format and bandwidth formula

Columns: `thread_count, stage, measured_time_s, measured_speedup, measured_efficiency, measured_bandwidth_GBs, measured_roofline_fraction`

```
bandwidth_GBs = bytes_moved / time_s / 1e9
             = (NX-2) × (NY-2) × (NZ-2) × NSTEPS × 56 / time_s / 1e9
             = 508³ × 100 × 56 / time_s / 1e9
             ≈ 734 / time_s   [GB/s]

roofline_fraction = bandwidth_GBs / STREAM_GBs
```

56 B/update = 6 reads × 8 B + 1 write × 8 B (6 face neighbours, no centre).

At perfect roofline (128T, STREAM = 231.5 GB/s): time ≈ 734 / 231.5 ≈ **3.17 s** for 100 steps.

STREAM values: 231.5 GB/s at 128T (graded), 116.0 GB/s at 64T, 246.2 GB/s at 32T (one-per-CCX).

`speedup = T(1,core) / T(P)` — use T(1,core) as reference for all rows including the extension row.

## REFLECTION guide

| Section | Key content required |
|---|---|
| 1 — Parallelisation strategy | Which loops, `collapse(2)` or `(3)`, double-buffer swap mechanism |
| 2 — Strong-scaling curve | Shape, cite times, name hardware boundary (CCD / socket BW / xGMI) |
| 3 — Extension choice and why | Why this extension is the right target for this kernel on Rome |
| 4 — Extension mechanism + delta | How the code changes, why it helps, measured before/after; if small delta, explain |
| 5 — Ice Lake counterfactual | Fewer NUMA domains (2 vs 8), no cross-socket xGMI → which extensions still matter |
| Reasoning (≤ 100 words) | What the extension changes about layout/distribution and why Rome specifically |

## Deliverables

All paths are relative to the repo root (this IS the assignment-3 repo — no subdirectory prefix).

| Path | What |
|---|---|
| `core/stencil.cpp` | Parallelised Jacobi step |
| `extension/<branch>/stencil_naive.cpp` (+ one more) | Before + after variants for chosen extension |
| `EXTENSION.md` | Structured YAML header + rationale |
| `answers.csv` | 15 MCQ answers |
| `tables.csv` | Times, speedups, efficiencies, bandwidth, roofline fractions |
| `REFLECTION.md` | Sections 1–5 ≥ 50 words each + Reasoning question |
| `perf-results-a3.json` | `hyperfine` output from CX3 `qsub evaluate.pbs` |

## Related

- [[../performance/Roofline Model]] — how to compute and interpret the A3 score.
- [[../performance/NUMA First Touch]] — extension option A.
- [[../performance/False Sharing]] — extension option B.
- [[../performance/SIMD]] — extension option C.
- [[../performance/Loop Transformations]] — `collapse(3)` for the stencil.
- [[Assessment Overview]] — rubric overview.
