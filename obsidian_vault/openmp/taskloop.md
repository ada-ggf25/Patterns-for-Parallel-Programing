# `taskloop`

`taskloop` splits an iteration space into ⌈n / grainsize⌉ *tasks* dispatched through the team's task pool. It is **not** a general replacement for `parallel for` — it is the right tool only in specific situations.

## Basic form

```cpp
#pragma omp parallel
{
#pragma omp single               // one thread issues the taskloop
    {
#pragma omp taskloop grainsize(8) default(none) shared(data, n)
        for (size_t i = 0; i < n; ++i) {
            process(data[i]);
        }
    }
}
```

Implicit `taskgroup` at the end — every chunk completes before the team moves on. Use `nogroup` to strip it. The combined `#pragma omp parallel masked taskloop` form collapses the boilerplate when you don't need the `single` for other reasons.

## When to use `taskloop` vs `parallel for`

For a *flat, top-level* loop, `parallel for` and `taskloop grainsize(C)` perform roughly equivalently. Prefer `parallel for` there — it's simpler (no `single` boilerplate).

Reach for `taskloop` only when:
- You're already **inside a `task` region** — a nested `parallel for` spins up a new team (often a perf cliff; nested parallelism is frequently disabled by default).
- Chunks must **interleave with sibling tasks** on the same team.
- Chunks need **`depend(...)` or `priority(...)`** clauses (OpenMP 5.1).

```cpp
// Same kernel, both paradigms — roughly equivalent for a top-level loop:
void apply_for(auto& v, auto f) {
#pragma omp parallel for default(none) shared(v) schedule(dynamic, 64)
    for (size_t i = 0; i < v.size(); ++i) v[i] = f(v[i]);
}

void apply_taskloop(auto& v, auto f) {
#pragma omp parallel
#pragma omp single
#pragma omp taskloop grainsize(64) default(none) shared(v)
    for (size_t i = 0; i < v.size(); ++i) v[i] = f(v[i]);
}
```

## Workload shape guide

| Workload | Best fit |
|---|---|
| Regular loop, uniform cost | `parallel for schedule(static)` |
| Regular loop, irregular cost | `parallel for schedule(dynamic, C)` |
| Loop *inside* an enclosing task region | `taskloop grainsize(C)` (composes; no nested team) |
| Recursive / divide-and-conquer | `task` + `taskwait` inside `single` |
| Pipeline / dependence graph | `task` + `depend(...)` |
| Producer / consumer over a queue | `task` + `priority` clause (5.1) |

A2 has you implement both a `parallel for` and a `taskloop` variant side by side — expect roughly equivalent performance; neither is rigged to win.

## `grainsize` tuning

| Symptom | Cause |
|---|---|
| Taskloop slower than serial at low P | grainsize too small — dispatch overhead dominates |
| Threads idle while one finishes at high P | grainsize too big — last task's tail dominates |
| Good 8× speedup but not 64× | task pool contention or imbalance |

For A2 (5000×2500 image, 100×100 tiles = ~1250 tiles): start at `grainsize(8)` (~150 tasks) and tune.

## Related

- [[Tasks]] — underlying task mechanism.
- [[Task Dependences]] — for pipeline shapes.
- [[../assessment/A2 Mandelbrot]] — A2 requires both `taskloop` and `parallel for` variants.
