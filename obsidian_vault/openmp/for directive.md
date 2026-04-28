# `#pragma omp for`

Splits the iterations of a `for` loop across the threads of an active team. No explicit index juggling — each thread automatically gets a contiguous chunk of `i`.

```cpp
#pragma omp parallel
{
    #pragma omp for
    for (int i = 0; i < n; ++i) {
        a[i] = b[i] + c[i];
    }
}
```

The combined shortcut is more common:

```cpp
#pragma omp parallel for
for (int i = 0; i < n; ++i) {
    a[i] = b[i] + c[i];
}
```

## What the compiler emits

OpenMP partitions the iteration space `[0, n)` and assigns one chunk to each thread. With 4 threads and `n = 16`, by default each thread gets 4 iterations:

| Thread | Iterations |
|---|---|
| 0 | 0, 1, 2, 3 |
| 1 | 4, 5, 6, 7 |
| 2 | 8, 9, 10, 11 |
| 3 | 12, 13, 14, 15 |

(The exact chunking depends on the `schedule` clause — see below.)

## Loop requirements (canonical form)

OpenMP can only parallelise a `for` loop in **canonical form**:

- A single integer (or pointer) loop variable.
- A constant or invariant lower and upper bound.
- A constant or invariant step (`++i`, `--i`, `i += k`, `i -= k`).
- No `break`, `goto` out of the loop, no `return`.

Iterations must be **independent** — no iteration may depend on another's writes (no `a[i] = a[i-1] + ...`). If they aren't, you have a race condition. See [[Variable Scoping]].

## The `schedule` clause

By default each thread gets a single contiguous chunk (`schedule(static)`). For uneven workloads, use `schedule(dynamic)` or `schedule(guided)`:

```cpp
#pragma omp parallel for schedule(static)              // default; equal contiguous chunks
#pragma omp parallel for schedule(static, 4)           // chunks of size 4, round-robin
#pragma omp parallel for schedule(dynamic)             // pull next iteration from queue
#pragma omp parallel for schedule(dynamic, 16)         // pull 16 at a time
#pragma omp parallel for schedule(guided)              // start big chunks, shrink
```

For the π example, every iteration costs the same, so `static` (the default) is fine. For irregular work — e.g. processing variable-length arrays per `i` — `dynamic` evens out load imbalance at the cost of slightly more overhead.

## Common clauses on `for`

- **`reduction(op:var)`** — accumulate per-thread values back into one. See [[reduction clause]].
- **`private(x)`, `firstprivate(x)`** — scoping. The loop counter is automatically private.
- **`nowait`** — skip the implicit barrier at the end of the loop (advanced).
- **`collapse(N)`** — collapse N nested loops into one iteration space.

## Related

- [[parallel directive]] — the surrounding fork.
- [[reduction clause]] — making accumulators safe.
- [[Variable Scoping]] — race conditions and how to avoid them.
- [[../examples/pi_openmp]] — the running example.
