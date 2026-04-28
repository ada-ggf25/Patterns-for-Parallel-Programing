# `#pragma omp parallel`

The fundamental OpenMP directive. It forks a team of threads; every line inside the `{ … }` runs on every thread.

```cpp
#include <iostream>
#include <omp.h>

int main() {
    #pragma omp parallel
    {
        std::cout << "Hello from thread " << omp_get_thread_num()
                  << " of " << omp_get_num_threads() << '\n';
    }
}
```

## What happens at runtime

- The number of threads is determined by `OMP_NUM_THREADS` (or `ompthreads=` in PBS, or the `num_threads(N)` clause).
- Each thread runs the body of the block. There is no automatic work-sharing — if you put a `for` loop inside a bare `parallel` region, **every thread runs the entire loop**.
- `omp_get_thread_num()` returns this thread's ID (0 to N-1).
- `omp_get_num_threads()` returns the team size.

## Output is unordered

The `Hello from thread ...` prints will appear in a non-deterministic order — that's the defining feature of multi-threading. If you care about ordering, you need explicit synchronisation (`#pragma omp critical`, `#pragma omp ordered`, etc.).

## Key clauses on `parallel`

- **`num_threads(N)`** — override `OMP_NUM_THREADS` for this region.
- **`if (cond)`** — fall back to single-threaded execution if `cond` is false (useful when N is too small to bother parallelising).
- **`shared(...)`, `private(...)`, `firstprivate(...)`, `default(...)`** — scoping clauses. See [[Variable Scoping]].
- **`reduction(op:var)`** — combine private values back to the master. See [[reduction clause]].

## Relationship to the for directive

Putting work-sharing inside a `parallel` block is the common case. The combined `#pragma omp parallel for` is shorthand for:

```cpp
#pragma omp parallel
{
    #pragma omp for
    for (...) ...
}
```

See [[for directive]].

## Related

- [[Fork Join Model]] — the execution model.
- [[for directive]] — work-sharing within a parallel region.
- [[Variable Scoping]] — what `shared` / `private` mean.
- [[OMP Environment Variables]] — controlling thread count.
