# A3 — 3D Jacobi Stencil (40 pts)

Parallelise a bandwidth-bound 7-point 3D Jacobi stencil and then apply one of three hardware-level optimisations as an extension.

## Kernel (A3-core)

7-point stencil over a 3D grid for `NSTEPS` timesteps:

```cpp
u_next[i,j,k] = (u[i,j,k]
    + u[i+1,j,k] + u[i-1,j,k]
    + u[i,j+1,k] + u[i,j-1,k]
    + u[i,j,k+1] + u[i,j,k-1]) * (1.0 / 7.0);
```

**OI ≈ 0.14 FLOPs/byte → bandwidth-bound, well below Rome's ridge at 18.7.**

The canonical parallelisation:

```cpp
#pragma omp parallel for collapse(3) default(none) shared(u, u_next, NX, NY, NZ)
for (size_t i = 1; i < NX-1; ++i)
    for (size_t j = 1; j < NY-1; ++j)
        for (size_t k = 1; k < NZ-1; ++k)
            u_next[idx(i,j,k)] = stencil(u, i, j, k);
```

`collapse(3)` distributes all `(NX-2)×(NY-2)×(NZ-2)` iterations — essential when any single dimension is small.

## NUMA-aware initialisation

**Critical for A3-core performance:** initialise `u` in the same parallel-for pattern you'll use for computation:

```cpp
// BAD — all pages on master's NUMA node → 7/8 cross-socket at 128 threads
for (size_t i = 0; i < NX*NY*NZ; ++i) u[i] = 0.0;

// GOOD — each thread first-touches its own pages
#pragma omp parallel for default(none) shared(u)
for (size_t i = 0; i < NX*NY*NZ; ++i) u[i] = 0.0;
```

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
// before (serial init)
for (size_t i = 0; i < n; ++i) u[i] = 0.0;

// after (parallel first-touch)
#pragma omp parallel for default(none) shared(u, n)
for (size_t i = 0; i < n; ++i) u[i] = 0.0;
```

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

Annotate the innermost stencil loop with `simd` + `aligned` + `safelen`.

```cpp
// after
#pragma omp simd aligned(u, u_next : 64) safelen(8)
for (size_t k = 1; k < NZ-1; ++k)
    u_next[idx(i,j,k)] = stencil(u, i, j, k);
```

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

## Justification (≤ 200 words)
<your prose>
```

`delta_percent = (before - after) / before × 100`. Grader checks this is consistent with your reported times within ±10 %.

## Diagnostic decision tree

| Symptom | Likely cause |
|---|---|
| 1.0× speedup at any P | Pragma typo, serial init, code actually serial |
| 4× at P=16, then plateau | Serial init → all pages on one NUMA domain |
| 8× at P=64, no gain at P=128 | Single socket bandwidth saturated |
| Large run-to-run variance | No warm-up or cold cache |

## Deliverables

| Path | What |
|---|---|
| `assignment-3/core/stencil.cpp` | Parallelised Jacobi step |
| `assignment-3/extension/<branch>/...` | Before + after variants |
| `assignment-3/EXTENSION.md` | Structured header + justification |
| `assignment-3/answers.csv` | 15 MCQ answers |
| `assignment-3/tables.csv` | Times + speedups + efficiencies + roofline fractions |
| `assignment-3/REFLECTION.md` | Required headers, ≥ 50 words per section |
| `assignment-3/perf-results-a3.json` | `hyperfine` output from CX3 |

## Related

- [[../performance/Roofline Model]] — how to compute and interpret the A3 score.
- [[../performance/NUMA First Touch]] — extension option A.
- [[../performance/False Sharing]] — extension option B.
- [[../performance/SIMD]] — extension option C.
- [[../performance/Loop Transformations]] — `collapse(3)` for the stencil.
- [[Assessment Overview]] — rubric overview.
