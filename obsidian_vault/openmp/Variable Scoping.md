# Variable Scoping

Inside a parallel region, every variable has a *data sharing attribute*: shared between all threads, or private to each. Get this wrong and you have a race condition.

## The four clauses

| Clause | Meaning |
|---|---|
| `shared(x)` | All threads see the same `x`. (Default for variables declared outside the region.) |
| `private(x)` | Each thread gets its own uninitialised copy. |
| `firstprivate(x)` | Each thread gets its own copy, initialised from the master's value. |
| `reduction(op:x)` | Private + combined at the end. See [[reduction clause]]. |

There's also `lastprivate(x)` — like `private` but the value of the *sequentially last* iteration is copied out at the end. Less commonly needed.

## The default is shared (and that's the problem)

```cpp
double sum = 0.0;
#pragma omp parallel for                // sum is shared by default → RACE
for (int i = 0; i < n; ++i) sum += a[i];
```

A variable declared outside the parallel region is **shared by default**. That's convenient most of the time but dangerous for accumulators — the canonical case is the example above, fixed by [[reduction clause]].

You can flip the default with `default(none)`, which forces you to scope every variable explicitly:

```cpp
#pragma omp parallel for default(none) shared(a, n) reduction(+:sum)
for (int i = 0; i < n; ++i) sum += a[i];
```

This is verbose but catches scoping bugs at compile time. Recommended for non-trivial loops.

## Loop variables are special

The loop counter of a `#pragma omp for` (`i` in `for (int i = …)`) is **automatically private**. You don't have to declare it. If you accidentally make it shared (e.g. by declaring it outside the loop), every thread fights over it and the loop produces garbage.

## Mental check before you parallelise

For each variable touched inside the loop, ask: "Is this written by more than one thread?" If yes, decide:

- It's an accumulator — use `reduction`.
- It's a per-thread workspace — use `private` (or `firstprivate` if it has an initial value to inherit).
- It's read-only — leave it `shared`.
- Two iterations both write to the same `a[i]` for the same `i` — your loop isn't actually parallel.

## What a race condition looks like

```cpp
double sum = 0.0;
#pragma omp parallel for
for (int i = 0; i < 1000000; ++i) sum += 1.0;
// Expected: 1000000.0
// Actual: maybe 998421.0, maybe 1000000.0, maybe 1000001.5 — different each run.
```

A race condition produces results that **change between runs**. If you see flaky timing-dependent answers from a parallel program, scoping is almost always the cause.

## Related

- [[reduction clause]] — the most common fix.
- [[parallel directive]] — where these clauses are written.
- [[for directive]] — the loop counter privacy rule.
- [[OpenMP Pitfalls]] — concrete examples of scoping mistakes.
