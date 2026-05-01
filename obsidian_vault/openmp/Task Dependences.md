# Task Dependences

`depend` clauses on tasks let the runtime build a dependency graph (DAG) and schedule tasks respecting partial ordering — without explicit `taskwait` after every step.

## `depend` clause semantics

| Clause | Meaning |
|---|---|
| `depend(out: x)` | "I write `x`" — must finish before any later `in: x` or `inout: x` on the same address. |
| `depend(in: x)` | "I read `x`" — waits for the most recent prior `out: x` or `inout: x`. |
| `depend(inout: x)` | Read-write — waits for prior writers and blocks subsequent readers. |

Address identity is what matters: two tasks listing the **same variable** in conflicting clauses are ordered; tasks on different variables are not and may run in parallel.

## Example: A → B chain + independent D, fan-in at R

```cpp
int a = 0, b = 0, d = 0, r = 0;

#pragma omp task depend(out: a)        // Task A writes a
{ a = compute_a(); }

#pragma omp task depend(in: a, out: b) // Task B reads a, writes b
{ b = compute_b(a); }

#pragma omp task depend(out: d)        // Task D is independent of A, B
{ d = compute_d(); }

#pragma omp task depend(in: b, in: d, out: r)  // R reads b and d — fan-in
{ r = compute_r(b, d); }
```

The runtime builds the DAG:

```
A ──▶ B ─┐
          ▼
D ────────▶ R
```

A and D run in parallel. B waits for A. R waits for both B and D.

## Why this is better than manual `taskwait`

Manual `taskwait` after every task serialises the chain:

```cpp
// BAD — forces serial execution
#pragma omp task { a = f(); }
#pragma omp taskwait
#pragma omp task { b = g(a); }
#pragma omp taskwait
```

With `depend`, the runtime only enforces the actual ordering constraints — independent tasks overlap freely.

## Inside a `parallel` / `single` block

```cpp
#pragma omp parallel
{
#pragma omp single
    {
        // All task spawning happens inside single
        #pragma omp task depend(out: a) { ... }
        #pragma omp task depend(in: a, out: b) { ... }
        #pragma omp task depend(in: b) { ... }
    }
    // implicit barrier at end of parallel waits for whole DAG
}
```

## Related

- [[Tasks]] — basic task lifecycle and `taskwait`.
- [[taskloop]] — convenience wrapper for loop-shaped task graphs.
- [[single and masked]] — why tasks are spawned inside `single`.
