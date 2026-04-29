# A2 REFLECTION

> Complete every section. CI will:
>
> 1. Verify all `## Section` headers are present.
> 2. Verify each section has **at least 50 words**.
>
> No automatic content grading: your prose is read by a human, and the
> reasoning question at the end is marked on a 0 / 0.5 / 1 scale. Numbers you
> quote here do **not** have to match canonical re-run timings exactly —
> queue variance is real. CHOICE.md is graded against your own committed
> `perf-results-a2.json`, not against canonical times.

## Section 1 — Task decomposition

Describe your task decomposition for the tasks variant. Per-pixel? Per-tile? What tile size (or `grainsize`) did you use and why? Minimum 50 words.

The tasks variant decomposes the upper half of the 5000×5000 grid into 100×100 pixel tiles (`TILE=100`), producing 50×25 = 1250 upper-half tiles in total. Each tile's escape-point count is computed by `count_tile_upper()` and doubled to account for real-axis conjugate symmetry.

I used `#pragma omp taskloop grainsize(1)` on the outer tile-row loop (50 iterations), which produces 50 tasks. Each task processes one outer-tile row sequentially across all 25 j-tile columns, handling 25 tiles and up to 250,000 pixels per task. I chose `taskloop` with `reduction(+:outside)` over a nested `#pragma omp task` approach for two reasons. First, the reduction clause gives each task a private accumulator that the runtime merges automatically at completion, so no `atomic` write is needed inside the tile loop. Second, the nested-task approach produced a persistent TSan false positive under Archer OMPT: the runtime's bget pool allocator reuses task-data heap blocks without OMPT callbacks, so Archer cannot establish the firstprivate-write happens-before relationship and fires a spurious data-race report. Switching to `taskloop` eliminated the false positive entirely.

## Section 2 — Comparison: parallel_for vs tasks

Looking at the measured times in your `tables.csv`, at which thread counts did tasks win, and at which did parallel_for? What does this pattern tell you about the workload shape of this Mandelbrot region? Minimum 50 words.

At 1 thread both variants are comparable: `parallel_for` takes 6.5982 s and `tasks` 6.4334 s — serial compute dominates and scheduling overhead is negligible. As thread count increases, `parallel_for` consistently wins by a growing margin. At 16T: `parallel_for` 0.4221 s vs `tasks` 0.5627 s (33% faster). At 64T: `parallel_for` 0.1144 s vs `tasks` 0.4409 s (3.9× faster). At 128T: `parallel_for` 0.0659 s vs `tasks` 0.4430 s (6.72× faster).

Tasks plateau sharply beyond 16T because `taskloop grainsize(1)` on the 50 outer-tile-row iterations creates only 50 tasks. With 128 threads, 78 threads receive no work and sit idle. `parallel_for schedule(dynamic, 1)` creates 5000 single-row chunks, so all 128 threads remain continuously busy, yielding a speedup of 100.12× at 128T with efficiency 0.78.

The workload is highly irregular: Mandelbrot boundary rows require close to MAXITER=1000 iterations per pixel, while interior rows escape quickly. This variance means `static` schedule performs poorly (0.196 s at 128T in the sweep), whereas `dynamic,1` absorbs the imbalance by assigning single rows on demand.

## Section 3 — Memory model considerations

Did your task decomposition require any explicit synchronisation beyond what `taskwait` / `taskgroup` provide? Did you see (or rule out) any potential race in a shared accumulator? Minimum 50 words.

Both variants use OpenMP's `reduction(+:outside)` clause to make the shared accumulator race-free without any explicit `atomic` or `critical` region.

In `parallel_for`, the clause is on `#pragma omp parallel for`: each thread gets a private copy of `outside` initialised to 0 and accumulates privately throughout the loop. The private copies are merged into the original at the implicit barrier after the loop — no flush or atomic is needed inside the loop body.

In the `tasks` variant, `reduction(+:outside)` is on `#pragma omp taskloop`: each task receives a private accumulator merged by the runtime at task completion. The `single` block has an implicit `taskwait` at its end, and the enclosing `parallel` region has an implicit barrier, so no explicit `taskwait` or `taskgroup` was needed.

`firstprivate(J_HALF)` copies the constant into each task's data environment by value. This is essential for two reasons: it ensures each task reads the correct value after the enclosing scope exits, and it eliminates the TSan false positive described in Section 1. In the original nested-`task` approach, `J_HALF` was passed as a shared pointer inside the task data block; Archer OMPT could not track the happens-before relationship through the runtime allocator and reported a spurious race. Switching to `taskloop` with `firstprivate(J_HALF)` produced zero TSan warnings under the exact CI flags (`halt_on_error=1`, Archer OMPT).

## Section 4 — Your recommendation

Your `CHOICE.md` picks one variant. Restate the recommendation here and summarise the *one* strongest piece of evidence for it (from your own measurements). If you picked the variant your own data showed as slower, your justification keyword (from the defensible-keyword enum) goes here. Minimum 50 words.

`CHOICE.md` recommends `parallel_for`. The single strongest piece of evidence is the 128-thread wall time from the formal CX3 Rome benchmark: `parallel_for` completes in 0.0659 s versus `tasks` at 0.4430 s, a 6.72× advantage at the target thread count. This result is reproducible — the Phase 3.5 sweep on a separate job produced 0.068 s versus 0.458 s, agreeing within 3%, confirming it is not a measurement artefact.

`parallel_for schedule(dynamic, 1)` achieves a speedup of 100.12× at 128T with efficiency 0.78, sustaining near-linear scaling across the full 128-core CX3 Rome node. The `tasks` variant is limited to 14.52× speedup and 0.11 efficiency at 128T because the task count (50) is below the thread count (128), leaving 78 threads idle for most of the run. Since the grader evaluates performance at 128 threads, `parallel_for` is the clear choice for this workload.

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain *when* a task-parallel decomposition beats a parallel-for for kernels with this class of workload.**

Task-parallel decomposition beats `parallel_for` when per-unit cost variance is high and the task count is sufficient to saturate all threads. For Mandelbrot, boundary tiles require up to MAXITER iterations while interior tiles escape immediately, creating extreme cost imbalance. A static `parallel_for` assigns contiguous rows at compile time, leaving threads that draw boundary rows overloaded while others idle. Fine-grained tasks dispatched dynamically send boundary tiles to whichever thread finishes first, eliminating the imbalance. The key condition is task count exceeding thread count; otherwise threads starve and the advantage disappears.
