# A2 — Mandelbrot Two-Variant Comparison (30 pts)

> **Active assessment.** See [[A2 Progress]] for the step-by-step checklist and current status.

Implement **two** parallel variants of Mandelbrot set membership counting over a 5000 × 5000 grid, benchmark both on CX3 Rome, and justify a production recommendation in `CHOICE.md`.

## Kernel

Estimate the area of the Mandelbrot set by counting how many of the NPOINTS × NPOINTS = 5000 × 5000 grid points escape within MAXITER = 1000 iterations.

**Symmetry shortcut** (already in serial + both scaffolds — do not remove):  
The Mandelbrot set is symmetric about the real axis: `mandel(c) = mandel(c̄)`. Only the upper half (`j ∈ [0, J_HALF)` where `J_HALF = NPOINTS / 2 = 2500`) is iterated; each escape there counts as +2 (one for the mirrored lower-half point). NPOINTS = 5000 is even, so the centre-row `if constexpr` branch is never taken — but the dead code must stay for the template to compile.

**Per-pixel cost:** 1 to 1000 escape iterations depending on position. Near the Mandelbrot boundary the cost varies by ~100×, creating severe load imbalance for static distribution. This is the core motivation for the tasks variant.

**Grid geometry:**
- NPOINTS = 5000, TILE = 100 (from `mandelbrot.h`)
- Real axis: `cr = -2.0 + 3.0 * i / NPOINTS` for `i ∈ [0, 5000)`
- Imag axis: `ci = -1.5 + 3.0 * j / NPOINTS` for `j ∈ [0, 2500)`
- Upper-half tile grid: 50 × 25 = **1250 tiles total**

**Expected output (from `expected_output.txt`):**
```
outside = 20807388
area = 7.490660
```

Both variants must produce this exact output at all thread counts. Correctness is checked by `bin/smart_diff.py`.

**What `smart_diff.py` actually checks:** token-wise comparison with float tolerance (`rtol=1e-6`, `atol=1e-8`). Both the integer count and the float area are compared with `np.isclose`. It checks prefix OR suffix — so extra debug output at the very beginning or very end of stdout will not fail the check. Any additional `printf` inside the computation will break it. Do not add debug prints inside the parallel loops or inside `main()` between the two existing `printf` calls.

## Files to edit

| File | What |
|---|---|
| `mandelbrot_for.cpp` | Implement `mandelbrot_parallel_for()` — edit the two nested loops |
| `mandelbrot_tasks.cpp` | Implement `mandelbrot_tasks()` — spawn tasks over the tile loops |

Do not edit `mandelbrot_serial.cpp`, `mandelbrot.h`, `bin/`, or `.github/workflows/`.

## Variant 1 — `mandelbrot_parallel_for()`

The scaffold in `mandelbrot_for.cpp` has a serial pixel loop. Add `#pragma omp parallel for` to parallelise it.

### Simplest correct approach (outer-loop only)

```cpp
long mandelbrot_parallel_for()
{
    long outside = 0;
    constexpr int J_HALF = NPOINTS / 2;

    #pragma omp parallel for reduction(+:outside) default(none) \
        shared(J_HALF) schedule(dynamic, TILE)
    for (int i = 0; i < NPOINTS; ++i) {
        for (int j = 0; j < J_HALF; ++j) {
            const double cr = -2.0 + (3.0 * static_cast<double>(i) / NPOINTS);
            const double ci = -1.5 + (3.0 * static_cast<double>(j) / NPOINTS);
            if (escape_iters(cr, ci) < MAXITER) {
                outside += 2;
            }
        }
    }
    // centre-row dead branch for even NPOINTS — leave as-is
    if constexpr (NPOINTS % 2 == 1) { /* ... */ }
    return outside;
}
```

### Why `reduction(+:outside)` with `+= 2`

Inside a `reduction(+:outside)` clause, each thread maintains a private copy initialised to 0. The `outside += 2` increments that private copy — no race. At the implicit barrier the private copies are summed into the shared `outside`. TSan-clean without any atomic.

### Schedule choice for `parallel_for`

| Schedule | Behaviour |
|---|---|
| `schedule(static)` | Even row-strip chunks; threads that hit boundary tiles wait while others idle |
| `schedule(dynamic, 1)` | One row per dispatch; fine balance but dispatch overhead at 128T |
| `schedule(dynamic, TILE)` | One tile-row (100 rows) per dispatch; good balance with low dispatch cost |

**Start with `schedule(dynamic, TILE)`.** Measure and compare against `schedule(static)` for REFLECTION evidence.

### `collapse(2)` option

Adding `collapse(2)` combines both loops into one iteration space of NPOINTS × J_HALF = 12.5 M iterations. With `schedule(dynamic, C)`, each chunk is C pixels — use C = TILE*TILE = 10000 to match tile granularity. This can improve balance at extreme thread counts but is harder to reason about.

## Variant 2 — `mandelbrot_tasks()`

The scaffold in `mandelbrot_tasks.cpp` already has:
- `count_tile_upper(i0, j0, j_half)` — counts escape points in one TILE×TILE block (upper half only, returns raw count without the ×2 mirror factor)
- A serial tile loop calling `outside += 2 * count_tile_upper(...)`

Replace the serial tile loop with task-parallel dispatch.

### Approach A — nested `#pragma omp task` with `atomic`

```cpp
long mandelbrot_tasks()
{
    long outside = 0;
    constexpr int J_HALF = NPOINTS / 2;

    #pragma omp parallel default(none) shared(outside, J_HALF)
    #pragma omp single
    {
        for (int i0 = 0; i0 < NPOINTS; i0 += TILE) {
            for (int j0 = 0; j0 < J_HALF; j0 += TILE) {
                #pragma omp task firstprivate(i0, j0) shared(outside, J_HALF)
                {
                    const long local = 2 * count_tile_upper(i0, j0, J_HALF);
                    #pragma omp atomic update
                    outside += local;
                }
            }
        }
    }  // implicit taskwait at end of single; implicit barrier at end of parallel
    // centre-row dead branch — leave as-is
    return outside;
}
```

Creates 1250 tasks (one per tile). `firstprivate(i0, j0)` captures the loop variables at task creation — critical to avoid the foot-gun where the loop advances before the task runs.

### Approach B — `#pragma omp taskloop` on the outer tile loop

```cpp
#pragma omp parallel default(none) shared(outside, J_HALF)
#pragma omp single
{
    #pragma omp taskloop grainsize(4) reduction(+:outside) default(none) firstprivate(J_HALF)
    for (int i0 = 0; i0 < NPOINTS; i0 += TILE) {          // 50 outer-tile iters
        for (int j0 = 0; j0 < J_HALF; j0 += TILE) {        // 25 inner — sequential per task
            outside += 2 * count_tile_upper(i0, j0, J_HALF);
        }
    }
}
```

`grainsize(4)` on 50 outer iterations → 12 tasks, each processing 4 tile-columns × 25 j-tiles = 100 tiles/task. The `reduction(+:outside)` on `taskloop` is TSan-clean; no atomic needed.

### Approach B′ — `taskloop` with flattened 1D index for finer granularity

```cpp
constexpr int N_TI = NPOINTS / TILE;          // 50
constexpr int N_TJ = (NPOINTS / 2) / TILE;    // 25
constexpr int N_TILES = N_TI * N_TJ;          // 1250

#pragma omp parallel default(none) shared(outside, J_HALF)
#pragma omp single
{
    #pragma omp taskloop grainsize(8) reduction(+:outside)
    for (int t = 0; t < N_TILES; ++t) {
        const int i0 = (t / N_TJ) * TILE;
        const int j0 = (t % N_TJ) * TILE;
        outside += 2 * count_tile_upper(i0, j0, J_HALF);
    }
}
```

`grainsize(8)` on 1250 → ~156 tasks. Better load balance at high thread counts than Approach B.

### TSan note for tasks

TSan + Archer OMPT (the CI environment) understands `#pragma omp atomic` and `taskloop reduction` as synchronisation points. It does **not** understand a raw unsynchronised write to `outside` inside a task — that is a data race and TSan will flag it. Always use `atomic update`, `in_reduction`, or `taskloop reduction` to accumulate.

## `grainsize` tuning guide

Applies to any taskloop or task-granularity decision:

| grainsize | Number of tasks on 1250 tiles (flattened) | Risk |
|---|---|---|
| 1 | 1250 | Task overhead dominates at low P |
| 8 | ~156 | Good starting point for A2 |
| 50 | 25 | Too coarse — last task tail visible at 128T |
| 1250 | 1 | Serial execution |

Rule of thumb: target **~10× tasks per thread** for good work-stealing balance. At 128T → target ~1280 tasks → grainsize(1) on 1250 tiles (or grainsize(8) on 1250-flattened is ~156 tasks → still 1 task/thread — probably too coarse). In practice, grainsize(4)–grainsize(16) on the flattened loop is the tuning range.

## `CHOICE.md` — format and grading

The grader parses the YAML front-matter deterministically. Fill all three numeric fields from your committed `perf-results-a2.json`.

```yaml
---
recommended: tasks        # one of: tasks | parallel_for
measured_tasks_s: 0.00    # your T(128) for mandelbrot_tasks from perf-results-a2.json
measured_for_s: 0.00      # your T(128) for mandelbrot_parallel_for
justification_keyword: irregular_load_balance   # see below — omit if recommending your faster variant
---
```

**Grading rule:** full CHOICE marks if `recommended` matches the variant your own `perf-results-a2.json` shows as faster at 128T. If you recommend the **slower** variant, you must supply one of these exact `justification_keyword` strings:

| Keyword | When appropriate |
|---|---|
| `irregular_load_balance` | Tasks enable dynamic load balance that for-static cannot match |
| `scales_better_at_128T` | Your data shows tasks win at 128T even if for wins at lower counts |
| `simpler_to_maintain` | Tile-based decomposition is architecturally cleaner (use only if data is very close) |
| `future_proof_for_dynamic_work` | Tasks compose with future dynamic/recursive extensions |

**Either variant can win.** The image region is chosen to be non-pathological — measure both and let your own data decide.

## `tables.csv` — format

Two rows per thread count (one per variant). Fill all three numeric columns for each row.

```
thread_count,variant,measured_time_s,measured_speedup,measured_efficiency
1,parallel_for,<T1_for>,1.00,1.00
1,tasks,<T1_tasks>,1.00,1.00
16,parallel_for,<T16_for>,<T1_for/T16_for>,<(T1_for/T16_for)/16>
16,tasks,<T16_tasks>,<T1_tasks/T16_tasks>,<(T1_tasks/T16_tasks)/16>
64,parallel_for,...
64,tasks,...
128,parallel_for,<T128_for>,...
128,tasks,<T128_tasks>,...
```

Arithmetic (grader checks within 2 %):
- `speedup(P) = T_variant(1) / T_variant(P)` — per-variant chain, not cross-variant
- `efficiency(P) = speedup(P) / P`
- P=1 rows: speedup = 1.00, efficiency = 1.00 (exact)

Use min times from the per-variant hyperfine JSON files on CX3:
```bash
python3 bin/hyperfine_min_time.py mandelbrot-parallel_for-1.json    # T(1) for_variant
python3 bin/hyperfine_min_time.py mandelbrot-tasks-128.json         # T(128) tasks_variant
```

## `perf-results-a2.json` — must commit

Unlike A1, `perf-results-a2.json` **must be committed**. The grader cross-checks your `CHOICE.md` header values (`measured_tasks_s`, `measured_for_s`) against this file within 5 %. Generated automatically by `evaluate.pbs`.

**File format** (generated by `evaluate.pbs`):
```json
{"results": [
  {"thread_count": 1,   "variant": "parallel_for", "time_s": 5.123},
  {"thread_count": 16,  "variant": "parallel_for", "time_s": 0.412},
  {"thread_count": 64,  "variant": "parallel_for", "time_s": 0.109},
  {"thread_count": 128, "variant": "parallel_for", "time_s": 0.063},
  {"thread_count": 1,   "variant": "tasks",        "time_s": 5.201},
  {"thread_count": 16,  "variant": "tasks",        "time_s": 0.389},
  {"thread_count": 64,  "variant": "tasks",        "time_s": 0.101},
  {"thread_count": 128, "variant": "tasks",        "time_s": 0.057}
]}
```

The `measured_tasks_s` in `CHOICE.md` must match `time_s` for `variant: "tasks"`, `thread_count: 128` — within 5 %. Same for `measured_for_s` for `variant: "parallel_for"` at 128T.

The 8 individual per-variant JSON files (`mandelbrot-parallel_for-1.json`, etc.) are intermediate; only `perf-results-a2.json` is read by the grader. Commit `perf-results-a2.json`; the individual files are optional.

```bash
# Commit after CX3 benchmark:
git add perf-results-a2.json tables.csv CHOICE.md
git commit -m "bench: fill tables.csv and CHOICE.md from CX3 Rome run"
```

## `REFLECTION.md` sections guide

CI checks headers present + each `## Section` body ≥ 50 words (HTML comments stripped). No content auto-scoring — a human reads it.

### Section 1 — Task decomposition

Cover:
1. What granularity you chose (per-pixel vs per-tile) and why
2. Tile size (100×100) — given by `TILE` constant; justify why this is a good size for the workload
3. `grainsize` value chosen and how you arrived at it
4. Whether you used nested `task` or `taskloop` and why

### Section 2 — Comparison: parallel_for vs tasks

Cover:
1. At which thread counts tasks won and at which parallel_for won (cite your `tables.csv`)
2. The pattern this reveals about the workload shape (boundary vs interior tiles)
3. Why static for scheduling is particularly hurt by the Mandelbrot boundary irregularity

### Section 3 — Memory model considerations

Cover:
1. How you ensured the shared `outside` accumulator has no data race
2. Which synchronisation construct you used (`atomic update`, `taskloop reduction`, `in_reduction`)
3. Whether `taskwait` / `taskgroup` was needed and why (or why not)
4. Why `firstprivate` on tile origin variables is essential in the nested-task approach

### Section 4 — Your recommendation

Restate the `CHOICE.md` recommendation and cite the strongest measurement supporting it (e.g., "tasks at 128T: X s vs parallel_for: Y s"). If recommending the slower variant, name the justification keyword and explain it.

### Reasoning question (≤ 100 words, instructor-marked)

**"In at most 100 words, explain when a task-parallel decomposition beats a parallel-for for kernels with this class of workload."**

Focus on: irregular per-tile cost → static for leaves boundary threads overloaded → dynamic task scheduling distributes tiles on demand → load balance advantage proportional to cost variance. Cite one measured data point.

## MCQ study guide (answers.csv, q01–q15)

> **Warning — q02 annotation in `questions.md`:** After the q02 answer choices, the file contains the text `*C — same as above; master is not a task.*` This is a leaked author annotation explaining why option C is wrong — **it does not mean the answer is C**. Read it as "C is wrong because master is not a task." The correct answer is the option that creates an async task.

| Question topic | Vault note |
|---|---|
| Why tasks beat static for irregular cost | [[../openmp/Tasks]], [[../openmp/Schedules]] |
| Which directive creates an async task (see q02 warning above) | [[../openmp/Tasks]] |
| `taskwait` scope (direct children only) | [[../openmp/Tasks]] |
| `taskloop` for tile-granularity load balancing | [[../openmp/taskloop]] |
| Uniform tiles: which variant has lower overhead | [[../openmp/taskloop]], [[../performance/Six Sources of Overhead]] |
| Why `single` wraps task-spawn loops | [[../openmp/single and masked]], [[../openmp/Tasks]] |
| `depend(in/out)` semantics | [[../openmp/Task Dependences]] |
| Chained `depend` — ordering guarantee | [[../openmp/Task Dependences]] |
| Tasks slower at 128T — likely cause | [[../openmp/taskloop]], [[../performance/Six Sources of Overhead]] |
| `taskloop grainsize(64)` task count | [[../openmp/taskloop]] |
| OpenMP memory model — flush points | [[../openmp/Memory Model]] |
| `taskgroup` — waits for entire subtree | [[../openmp/Tasks]] |
| CHOICE.md mismatch — correct response | [[assessment/A2 Mandelbrot]] (this note) |
| `untied` tasks — migration behaviour | [[../openmp/Tasks]] |
| Speedup vs efficiency internal consistency | [[../performance/Performance Metrics]] |

## Scoring (30 pts)

| Component | Pts | Check |
|---|---|---|
| Build + TSan clean (both variants) | 3 | Clang-18 + TSan + Archer on CI at {1,2,4,8,16} |
| Correctness (both variants × 4 thread counts) | 6 | 0.75 pts × 8 cells; `smart_diff.py` on CX3 Rome |
| Reference-parallel-time of better variant at 128T | 8 | `min(1.0, T_ref(128) / min(T_for, T_tasks)) × 8`; correctness-gated |
| CHOICE.md evidence consistency | 4 | Deterministic parse; must match own `perf-results-a2.json` |
| `tables.csv` internal consistency | 2 | `speedup = T(1)/T(P)`, `eff = speedup/P` within 2 % |
| Style (clang-format / clang-tidy / cppcheck) | 2 | Lint workflow |
| MCQ (15 questions) | 2 | Auto-graded against `answers.csv` |
| REFLECTION format + completion | 1 | Header check + ≥ 50 words per Section |
| Reasoning question (instructor-marked) | 2 | Manual 0 / 1 / 2 |

### Reference-parallel-time scoring

$$\text{score} = \min\!\left(1.0,\; \frac{T_{\text{ref}}(128)}{\min(T_{\text{for}}(128),\; T_{\text{tasks}}(128))}\right) \times 8$$

The grader takes the **better** of your two variants. Both must be correct. `T_ref` is the instructor's reference solution time on Rome, published once at start of cohort.

## Common mistakes

| Mistake | Effect | Fix |
|---|---|---|
| No `firstprivate(i0, j0)` on nested task | Captures loop variable by reference; task reads advanced value → wrong count or TSAN race | Always `firstprivate` loop indices in task spawn |
| `outside += local` inside task without `atomic` | Data race — TSan reports it, correctness undefined | Use `#pragma omp atomic update` or `taskloop reduction(+:outside)` |
| Omitting `default(none)` | Silently shares all outer-scope vars; `openmp-use-default-none` clang-tidy check will also flag it → lint CI fails | Add `default(none)` and list each shared/firstprivate explicitly |
| `grainsize(1)` on pixel loop (1250 tiles but per-pixel) | 12.5 M tasks — dispatch overhead 100× compute | Use tile-level granularity, not pixel-level |
| Committing `build/` or binary artifacts | CI `generated-files-check` fails | Keep `.gitignore` covering `build/` |
| NOT committing `perf-results-a2.json` | CHOICE.md cross-check fails → 0 CHOICE marks | `git add perf-results-a2.json` after CX3 run |
| `measured_tasks_s` / `measured_for_s` in CHOICE.md don't match json (>5 % off) | Grader cross-check fails | Copy from `perf-results-a2.json` exactly |
| `schedule(runtime)` left in `parallel_for` without `OMP_SCHEDULE` set | Falls back to `static` silently on grader node → worst-case imbalance | Hardcode `schedule(dynamic, TILE)` or other winner |
| Extra `printf` debug lines between the two output lines in `main()` | Breaks `smart_diff.py` match — the expected tokens won't align to prefix or suffix | Remove all debug prints before final push |

## Related

- [[A2 Progress]] — step-by-step checklist and current status.
- [[A2 Benchmark Results]] — CX3 Rome measured times (populated after `evaluate.pbs` run).
- [[../openmp/Tasks]] — task lifecycle, `taskwait`, `taskgroup`, data environment.
- [[../openmp/taskloop]] — `grainsize`, when taskloop beats parallel for.
- [[../openmp/Task Dependences]] — `depend` clauses for MCQ q07/q08.
- [[../openmp/single and masked]] — why `single` is required for task-spawn loops.
- [[../openmp/Memory Model]] — flush points, atomic, task reduction.
- [[../openmp/critical and atomic]] — `atomic update` for safe accumulation.
- [[../openmp/Schedules]] — schedule selection for the `parallel_for` variant.
- [[../performance/Performance Metrics]] — speedup, efficiency, reference-parallel-time formula.
- [[../performance/Six Sources of Overhead]] — why high-P efficiency drops.
- [[Assessment Overview]] — 100-pt rubric, thread ladder, grading formulas.
