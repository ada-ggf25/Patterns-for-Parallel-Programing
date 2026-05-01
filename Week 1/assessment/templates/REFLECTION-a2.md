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

<!-- your answer here -->

## Section 2 — Comparison: parallel_for vs tasks

Looking at the measured times in your `tables.csv`, at which thread counts did tasks win, and at which did parallel_for? What does this pattern tell you about the workload shape of this Mandelbrot region? Minimum 50 words.

<!-- your answer here -->

## Section 3 — Memory model considerations

Did your task decomposition require any explicit synchronisation beyond what `taskwait` / `taskgroup` provide? Did you see (or rule out) any potential race in a shared accumulator? Minimum 50 words.

<!-- your answer here -->

## Section 4 — Your recommendation

Your `CHOICE.md` picks one variant. Restate the recommendation here and summarise the *one* strongest piece of evidence for it (from your own measurements). If you picked the variant your own data showed as slower, your justification keyword (from the defensible-keyword enum) goes here. Minimum 50 words.

<!-- your answer here -->

## Reasoning question (instructor-marked, ≤100 words)

**In at most 100 words, explain *when* a task-parallel decomposition beats a parallel-for for kernels with this class of workload.**

<!-- your answer here; 100 words max -->
