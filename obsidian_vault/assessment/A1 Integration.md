# A1 — Numerical Integration (20 pts)

Parallelise a composite-trapezoid integration kernel with **non-uniform per-iteration cost**, then analyse schedule performance across the canonical thread ladder.

## Kernel

Integrate `f(x)` over `[0, 1]` with N subintervals using the midpoint rule. `f(x)` has a deliberate **spike region** for `x ∈ [0.3, 0.4]` running ~10× slower than elsewhere — schedule choice is genuinely informative.

```cpp
double sum = 0.0;
#pragma omp parallel for default(none) shared(n, w) reduction(+:sum) schedule(???)
for (long long i = 1; i <= n; ++i) {
    const double x = w * (static_cast<double>(i) - 0.5);
    sum += f(x);   // f(x) is cheap outside [0.3, 0.4], expensive inside
}
```

## Skills exercised

- `parallel for` + `reduction(+:sum)` + `default(none)`
- Schedule selection: `static`, `dynamic, C`, `guided`
- Timing with `omp_get_wtime` + warm-up
- Reference-parallel-time reasoning

## Scoring (20 pts)

| Component | Pts | Check |
|---|---|---|
| Build + TSan clean | 2 | Clang-18 + TSan + Archer |
| Correctness at `{1, 16, 64, 128}` | 6 | 1.5 pts per thread count |
| Reference-parallel-time at 128T | 5 | `min(1.0, T_ref(128) / T_student(128)) × 5` |
| `tables.csv` internal consistency | 1 | `speedup = T(1)/T(P)`, `eff = speedup/P` within 2 % |
| Style (clang-format / clang-tidy / cppcheck) | 2 | Lint workflow |
| MCQ (15 questions) | 2 | Key comparison |
| REFLECTION format + completion | 1 | Header presence + ≥ 50 words per section |
| Reasoning question (instructor-marked) | 1 | Manual 0 / 0.5 / 1 |

## Schedule sweep methodology

For the spike workload, measure all three schedules at each thread count:

```cpp
const double t_static  = time(sum_static, n);       // schedule(static)
const double t_dynamic = time(sum_dynamic_64, n);   // schedule(dynamic, 64)
const double t_guided  = time(sum_guided, n);        // schedule(guided)
```

Record all three in `tables.csv`. The winner can differ by thread count — don't assume the same winner everywhere. `static` may win at low counts (less overhead); `dynamic` wins at high counts with the spike.

## Deliverables

| Path | What |
|---|---|
| `assignment-1/integrate.cpp` | Parallel implementation |
| `assignment-1/answers.csv` | 15 MCQ answers (one A/B/C/D per row) |
| `assignment-1/tables.csv` | Times + speedups + efficiencies at `{1, 16, 64, 128}` |
| `assignment-1/REFLECTION.md` | Required headers, ≥ 50 words per section |
| `assignment-1/perf-results-a1.json` | `hyperfine` output from CX3 self-benchmark |

## Common mistakes

- Forgetting `default(none)` → race on accumulator.
- Not warming up → first run looks 5–10× slower (inflates T(128)).
- Putting the same schedule for all thread counts without measuring.
- `tables.csv` where `efficiency = speedup` (forgot to divide by P).

## Related

- [[../openmp/Schedules]] — static / dynamic / guided in depth.
- [[../openmp/reduction clause]] — the canonical fix.
- [[../performance/Timing omp_get_wtime]] — warm-up, min-of-k.
- [[Assessment Overview]] — 100-pt rubric and grading formulas.
