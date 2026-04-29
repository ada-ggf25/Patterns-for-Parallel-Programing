# A2 — Multiple-choice questions

Fill in your answers in `answers.csv` (one letter per question, A/B/C/D).

## q01

The most common reason a task-parallel decomposition beats a static-scheduled `parallel for` for Mandelbrot image rendering is:

- A. Tasks use less memory than worksharing loops.
- B. Per-pixel (or per-tile) cost is highly variable; dynamic task scheduling balances load better.
- C. Tasks run on the GPU automatically.
- D. `#pragma omp for` cannot be used on 2D loops.

## q02

Which directive creates a task that executes *asynchronously* relative to the encountering thread?

- A. `#pragma omp task`
- B. `#pragma omp single`
- C. `#pragma omp master`
- D. `#pragma omp barrier`

*C — same as above; master is not a task.*

## q03

`taskwait` inside a region:

- A. Blocks the current thread until all child tasks spawned in the current task complete.
- B. Blocks until all tasks in the entire program complete.
- C. Cancels outstanding tasks.
- D. Is deprecated in OpenMP 5.1.

## q04

To parallelise a loop of independent tile computations with minimum code change while getting task-like load balancing, use:

- A. `#pragma omp parallel for schedule(static)`
- B. `#pragma omp taskloop`
- C. `#pragma omp sections`
- D. `#pragma omp atomic`

## q05

For an image where all tiles cost roughly the same (uniform divergence, e.g. far from the Mandelbrot boundary), which is most likely the *faster* variant on Rome?

- A. `parallel_for` — lower per-iteration overhead than tasking.
- B. `tasks` — always faster.
- C. They are always identical.
- D. Depends on compiler, not workload.

## q06

A common idiom for tasks over a loop: one thread enters `#pragma omp single` and spawns `#pragma omp task` per iteration. Why `single`?

- A. To ensure tasks are spawned exactly once, not once per thread.
- B. Because `single` runs faster than `master`.
- C. Because tasks can only be spawned from the master thread.
- D. Because `single` has no implicit barrier.

## q07

`#pragma omp task depend(in: x) depend(out: y)` establishes:

- A. A dependence chain — this task waits for any earlier sibling task with `depend(out: x)`, and any later sibling task with `depend(in: y)` will wait for it.
- B. A synchronisation barrier for all threads.
- C. A mutual-exclusion lock on `x` and `y`.
- D. Only a hint to the compiler; ignored at runtime.

## q08

In a task graph with `depend(in: a) depend(out: b)` followed by another task with `depend(in: b) depend(out: c)`, the second task:

- A. Must wait for the first to complete.
- B. May start immediately.
- C. Cannot be created until the first finishes.
- D. Starts only after all other tasks complete.

## q09

You observe your tasks variant is *slower* than the parallel_for variant on 128 threads. The most likely cause is:

- A. Grainsize too small — task overhead dominates useful work.
- B. Tasks can't use all 128 threads.
- C. OpenMP 5.1 tasks are slower than 4.5 tasks.
- D. The compiler disabled task support.

## q10

`taskloop grainsize(64)` on a 10000-iteration loop:

- A. Creates ~156 tasks, each covering 64 iterations.
- B. Creates 64 tasks total.
- C. Creates 10000 tasks.
- D. Creates one task per thread in the team.

## q11

Which statement about OpenMP's memory model is true?

- A. All writes are immediately visible to all threads.
- B. Writes become visible to other threads at implicit or explicit `flush` points; the boundaries of worksharing and synchronisation constructs are flush points.
- C. There is no memory model; the programmer is fully responsible.
- D. `flush` only works inside `#pragma omp critical`.

## q12

A recursive Fibonacci implemented with tasks typically uses `#pragma omp taskgroup` to:

- A. Wait for all descendant tasks (including those spawned by children) before proceeding.
- B. Group tasks for logging.
- C. Force tasks to execute in spawn order.
- D. Pin tasks to specific threads.

## q13

Your `CHOICE.md` says `recommended: tasks` but CI measured `parallel_for` as the faster variant at 128 threads. For full marks under the rubric, you must:

- A. Change the recommendation to `parallel_for`.
- B. Leave it as-is; the rubric rewards tasks.
- C. Either switch the recommendation, or cite a defensible keyword (e.g. `scales_better_at_128T` or `future_proof_for_dynamic_work`) AND include supporting measured evidence.
- D. Remove the CHOICE.md file.

## q14

`untied` tasks:

- A. May migrate between threads mid-execution; `tied` (the default) may not.
- B. Always run faster than tied tasks.
- C. Cannot be used with `depend`.
- D. Are the only way to use `taskyield`.

## q15

Internal consistency check in your prediction-vs-measurement table: you recorded `measured_speedup = 6.67` and `measured_efficiency = 0.9` at 8 threads. Are these consistent?

- A. Yes.
- B. No — efficiency should be speedup / P = 6.67 / 8 = 0.83, not 0.9.
- C. Can't tell without seeing the raw time.
- D. Both are wrong; should be `speedup × P`.
