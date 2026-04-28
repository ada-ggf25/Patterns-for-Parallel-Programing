# `taskloop`

`taskloop` is the convenience form of task-based loop parallelism. It collapses the `parallel` + `single` + manual `task` spawn pattern into one directive and adds automatic `grainsize` control.

## Basic form

```cpp
#pragma omp parallel
{
#pragma omp single
    {
#pragma omp taskloop grainsize(8) default(none) shared(data, n)
        for (size_t i = 0; i < n; ++i) {
            process(data[i]);
        }
    }
}
```

The runtime slices `[0, n)` into chunks of `grainsize` iterations and queues each as a task. Team members pick up tasks dynamically — late starters help finish even when per-item cost varies.

## `taskloop` vs `parallel for`

```cpp
// Same kernel, two paradigms:
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

| Criterion | `parallel for` | `taskloop` |
|---|---|---|
| Per-iter overhead | Lower | Higher (task dispatch) |
| Load balance | Depends on schedule | Dynamic by default |
| Recursive composability | Hard | Natural |

## Workload shape guide

| Workload | Best fit |
|---|---|
| Regular loop, uniform cost | `parallel for schedule(static)` |
| Regular loop, irregular cost | `parallel for schedule(dynamic, C)` |
| Regular loop, very irregular | `taskloop grainsize(C)` |
| Recursive / divide-and-conquer | `task` + `taskwait` inside `single` |
| Pipeline / dependence graph | `task` + `depend(...)` |

A2's Mandelbrot sits in the "very irregular" row — tile cost varies dramatically across the image.

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
