# PPP-OpenMP rubric (canonical, 100 pts)

This is the canonical rubric. The grader's `W` weights dict
(`evaluate-assessment-artifacts.py` in the private grader repo) asserts
`sum_total == 100` at module import; any change here must mirror in the
grader.

## Top-level allocation

| Component | Pts |
|---|---|
| A1 — Numerical integration | 20 |
| A2 — Mandelbrot two-variant comparison | 30 |
| A3-core — 3D Jacobi stencil | 25 |
| A3-extension — chosen branch | 15 |
| Hygiene (build clean + lint + README) | 10 |
| **Total** | **100** |

Commit-history quality contributes **0 pts** — flag-only for instructor
academic-integrity review (see `assessment/handouts/commit-history-guidance.md`).

## A1 — Numerical integration (20 pts)

| Component | Pts | Source |
|---|---|---|
| Build + TSan clean | 2 | Clang-18 + TSan + Archer (formative GH Actions + canonical CX3) |
| Correctness (graduated, 4 thread counts) | 6 | `smart_diff.py` at `{1, 16, 64, 128}`; 1.5 pts each |
| Reference-parallel-time perf | 5 | `min(1.0, T_ref(128) / T_student(128)) × 5`. Correctness-gated. |
| `tables.csv` internal consistency | 1 | `speedup = T(1)/T(P)` and `efficiency = speedup/P` within 2 %. No canonical cross-check. |
| Style (clang-format / clang-tidy / cppcheck) | 2 | Lint workflow |
| MCQ | 2 | 15 multi-choice questions; deterministic key-comparison |
| REFLECTION format + completion | 1 | Header presence + ≥ 50 words per section. No numerical cross-check. |
| Reasoning question (instructor-marked) (≤ 100 words) | 1 | Manual 0 / 0.5 / 1 |

## A2 — Mandelbrot two-variant comparison (30 pts)

| Component | Pts | Source |
|---|---|---|
| Build + TSan clean (both variants) | 3 | Both targets compile cleanly under Clang-18 + TSan + Archer |
| Correctness (both variants × 4 thread counts) | 6 | `smart_diff.py`; 0.75 pts × 8 cells |
| Reference-parallel-time perf of better variant | 8 | `min(1.0, T_ref(128) / min(T_for, T_tasks)) × 8`. Correctness-gated. |
| CHOICE.md evidence consistency | 4 | Recommendation matches the variant the student's *own* `perf-results-a2.json` shows faster, OR matches the slower variant with a defensible keyword. **Canonical times are not consulted.** |
| `tables.csv` internal consistency | 2 | Per-variant `speedup` and `efficiency` self-consistent within 2 %. |
| Style | 2 | Lint workflow |
| MCQ | 2 | 15 questions on tasks-vs-loop trade-offs + memory model |
| REFLECTION format + completion | 1 | Format check only |
| Reasoning question (instructor-marked) | 2 | Manual 0 / 1 / 2 |

## A3-core — 3D Jacobi stencil (25 pts)

| Component | Pts | Source |
|---|---|---|
| Build + TSan clean | 2 | Clang-18 + TSan + Archer |
| Correctness (graduated, 4 thread counts) | 8 | `smart_diff.py` on output checksum at `{1, 16, 64, 128}`; 2 pts each |
| Roofline performance (memory-bound, validated) | 6 | `achieved_GBs(128) / measured_STREAM(128)`. Graduated: ≥ 0.70 → full; ≥ 0.50 → 4.2; ≥ 0.30 → 2.4; ≥ 0.15 → 0.9; else 0. Correctness-gated. |
| `tables.csv` internal consistency | 2 | Per-row `speedup` and `efficiency` self-consistent within 2 %. |
| Style | 2 | Lint workflow |
| MCQ | 2 | 15 questions; only core-relevant ones scored at this stage |
| REFLECTION (core sections) format + completion | 1 | Format check |
| Reasoning question (instructor-marked) | 2 | Manual 0 / 1 / 2 |

## A3-extension — chosen branch (15 pts)

Student picks ONE of `numa_first_touch`, `false_sharing`, or `simd` and
declares the choice in `EXTENSION.md`'s structured header.

| Component | Pts | Source |
|---|---|---|
| Implementation | 7 | Two compiled variants (before / after) build, run, produce correct output. The mechanism is actually used. |
| Soft-threshold delta | 5 | Reads `before_time_s` and `after_time_s` from `EXTENSION.md` (consistent with `delta_percent` within 10 % — internal-consistency only). Thresholds: NUMA / false-sharing → `delta_percent ≥ 15` full; `≥ 5` half; else attempt-credit (1 pt). SIMD → `ratio ≥ 1.2×` full; `≥ 1.05×` half. |
| Reasoning question (instructor-marked) | 3 | Manual 0 / 1 / 2 / 3 on a fixed prompt about *what* the extension does and *why* it helps on Rome specifically. |

## Hygiene (10 pts)

| Component | Pts | Source |
|---|---|---|
| Build hygiene (no warnings at `-Wall -Wextra -Wpedantic`) | 3 | Compiler output |
| Lint tools (clang-format, clang-tidy, cppcheck) | 4 | Lint workflow; ~1.3 pts each |
| README / English readability | 3 | Manual 0 / 1 / 2 / 3 |

## Frozen reference times

The reference target times for A1 + A2 are measured once at the start of
the cohort by the instructor using the private reference parallel
solutions on Rome and are then **published** here.

```
A1 reference times on Rome (measured YYYY-MM-DD):
  T_ref(1)   = TBD s
  T_ref(16)  = TBD s
  T_ref(64)  = TBD s
  T_ref(128) = TBD s

A2 reference times on Rome (measured YYYY-MM-DD), published as min(parallel_for, tasks):
  T_ref(1)   = TBD s
  T_ref(16)  = TBD s
  T_ref(64)  = TBD s
  T_ref(128) = TBD s
```

These numbers are frozen for the cohort once published. Update at the
start of each cohort by re-running `reference/reference_eval.pbs` in the
private grader repo and copying the relevant fields here.

## Performance scoring formulae

```
ref_parallel_pts(t_student, t_ref, full_pts) =
    min(1.0, t_ref / t_student) × full_pts        # A1 + A2

roofline_pts_a3(achieved_GBs, ceiling_GBs, full_pts) =
    if   achieved / ceiling ≥ 0.70 then full_pts
    elif achieved / ceiling ≥ 0.50 then full_pts × 0.70
    elif achieved / ceiling ≥ 0.30 then full_pts × 0.40
    elif achieved / ceiling ≥ 0.15 then full_pts × 0.15
    else 0
```

Both are correctness-gated: any thread-count correctness fail → 0 perf.

## Hardware (Rome — measured 2026-04-24)

- Dual-socket EPYC 7742 = 128 physical cores, 8 NUMA domains (NPS=4 per socket, 16 cores per domain), SMT off.
- Peak DP 4608 GFLOPs (AVX2 FMA).
- STREAM triad ceiling 246.2 GB/s (32 threads, one per CCX); 231.5 GB/s at 128 threads full-node; 116.0 GB/s on one socket (close+cores). Measured 2026-04-26 v3 sweep with STREAM_ARRAY_SIZE=800M, both GCC 13.3 and Clang 18 (compilers tied).
- Canonical thread ladder `{1, 16, 64, 128}`.
