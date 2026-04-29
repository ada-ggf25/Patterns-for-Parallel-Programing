# A2 — Mandelbrot Two-Variant Comparison (30 pts)

Implement **two** parallel variants of Mandelbrot set membership counting, benchmark both, and justify a production recommendation.

## Kernel

Count Mandelbrot-set membership over a 5000 × 5000 grid (post-symmetry: 5000 × 2500 sampled, doubled). Per-pixel cost: 1 to 1000 escape-time iterations depending on position — **highly irregular**.

Per-tile cost ratio can exceed 100×. The image region is irregular but not pathological — neither variant is rigged to win; measure both and let your own data decide.

## Two required variants

### Variant 1: `parallel for collapse(2)`

```cpp
#pragma omp parallel for collapse(2) default(none) shared(tiles, ...)
for (int ti = 0; ti < tile_rows; ++ti)
    for (int tj = 0; tj < tile_cols; ++tj)
        process_tile(tiles[ti][tj]);
```

### Variant 2: `taskloop grainsize(C)`

```cpp
#pragma omp parallel
#pragma omp single
#pragma omp taskloop grainsize(8) default(none) shared(tiles, ...)
for (int t = 0; t < n_tiles; ++t)
    process_tile(tiles[t]);
```

The `parallel for` approach distributes tiles statically (or dynamically with a chunk); `taskloop` uses work-stealing with `grainsize`-sized task chunks.

## `CHOICE.md` format

```yaml
---
recommended: tasks                     # or: parallel_for
measured_for_s: 0.49
measured_tasks_s: 0.31
justification_keyword: irregular_load_balance   # only if recommending the slower variant
---
```

The grader parses this header. Your recommendation passes if it matches the variant your **own** committed `perf-results-a2.json` shows as faster, OR if you recommend the slower one with a defensible keyword (`irregular_load_balance`, `simpler_code`, etc.).

**Either variant can be the right answer.** Expect roughly equivalent performance — read your own measurements and let the data drive your `CHOICE.md`.

## Scoring (30 pts)

| Component | Pts | Check |
|---|---|---|
| Build + TSan clean (both variants) | 3 | Clang-18 + TSan + Archer |
| Correctness (both variants × 4 thread counts) | 6 | 0.75 pts × 8 cells |
| Reference-parallel-time of better variant at 128T | 8 | `min(1.0, T_ref(128) / min(T_for, T_tasks)) × 8` |
| CHOICE.md evidence consistency | 4 | Matches your own `perf-results-a2.json` |
| `tables.csv` internal consistency | 2 | Per-variant self-consistent within 2 % |
| Style | 2 | Lint workflow |
| MCQ (15 questions) | 2 | Tasks vs loop trade-offs + memory model |
| REFLECTION format + completion | 1 | Format check |
| Reasoning question (instructor-marked) | 2 | Manual 0 / 1 / 2 |

## `grainsize` tuning

For A2: tiles are 100 × 100 px, ~1250 total tiles. Start at `grainsize(8)` ≈ 150 tasks. Too small → task dispatch overhead dominates at low P. Too large → last task's tail dominates at high P.

| Symptom | Likely cause |
|---|---|
| Taskloop slower than serial | grainsize too small |
| Speedup plateaus around 4–8× regardless of P | Last large task dominates; reduce grainsize |

## Deliverables

| Path | What |
|---|---|
| `assignment-2/mandelbrot_for.cpp` | parallel-for variant |
| `assignment-2/mandelbrot_tasks.cpp` | tasks variant |
| `assignment-2/CHOICE.md` | Structured header + ≤ 200 word justification |
| `assignment-2/answers.csv` | 15 MCQ answers |
| `assignment-2/tables.csv` | Per-variant per-thread-count times |
| `assignment-2/REFLECTION.md` | Required headers, ≥ 50 words per section |
| `assignment-2/perf-results-a2.json` | `hyperfine` output from CX3 |

## Related

- [[../openmp/taskloop]] — grainsize tuning, tasks vs for.
- [[../openmp/Tasks]] — the underlying task mechanism.
- [[Assessment Overview]] — rubric and grading formulas.
