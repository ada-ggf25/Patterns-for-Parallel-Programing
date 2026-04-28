# Variable Scoping

Inside a parallel region, every variable has a *data sharing attribute*: shared between all threads, or private to each. Get this wrong and you have a race condition.

## The data-sharing clauses

| Clause | Meaning |
|---|---|
| `shared(x)` | All threads see the same `x`. (Default for variables declared outside the region.) |
| `private(x)` | Each thread gets its own uninitialised copy. |
| `firstprivate(x)` | Each thread gets its own copy, initialised from the master's value. |
| `lastprivate(x)` | Like `private` but the sequentially-last iteration's value is copied out at exit. |
| `reduction(op:x)` | Private + combined at the end. See [[reduction clause]]. |
| `default(none)` | Forces you to declare every variable explicitly — catches scoping bugs at compile time. |
| `default(shared)` | Implicit default — the foot-gun this course rejects. |

## `default(none)` discipline

**Strongly recommended for every parallel region in this course.** `default(none)` makes the compiler error if any captured variable is not explicitly scoped — turning accidental sharing from a runtime mystery into a compile-time error:

```cpp
#pragma omp parallel for default(none) shared(a, n) reduction(+:sum)
for (size_t i = 0; i < n; ++i) sum += a[i];
```

Reading a region with `default(none)` is auditable: every variable's role is declared on the line.

## `firstprivate` — when right vs wrong

**Right**: each thread genuinely mutates a per-iteration scratch initialised from an outer value:

```cpp
int base = 42;
#pragma omp parallel for default(none) firstprivate(base) shared(a, n)
for (size_t i = 0; i < n; ++i) {
    base += i;        // safe — modifying own local copy
    a[i] = base;
}
```

**Wrong**: using `firstprivate` for a read-only outer value that should simply be `shared`. This masks bugs (the variable looks correct because each thread has its own copy) and misleads readers about the intent:

```cpp
// BAD — outer is read-only; firstprivate is misleading
#pragma omp parallel for default(none) firstprivate(outer) shared(a, n)
for (size_t i = 0; i < n; ++i) a[i] = outer;  // should be shared(outer)
```

Rule: if you never write to the variable inside the region, it should be `shared`, not `firstprivate`.

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
