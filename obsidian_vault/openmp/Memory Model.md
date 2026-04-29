# OpenMP Memory Model

OpenMP 5.1 inherits the C++11 memory model and adds explicit `flush`, `acquire`, and `release` semantics. Understanding this is essential for correctness in any pattern beyond a simple reduction.

## The problem: writes don't auto-propagate

```cpp
// Thread A:              // Thread B:
data = 42;                while (!ready) ;
ready = true;             printf("%d\n", data);  // may print 0!
```

Without synchronisation, the compiler and CPU may:
- Reorder `data` and `ready` writes (compiler or CPU).
- Keep `ready = true` in a store buffer before it reaches cache.
- Let thread B observe `ready = 1` before `data` is visible.

This program can print 0, 42, or anything else. It's undefined behaviour.

## When writes become visible

A thread's write becomes visible to other threads at:

1. An explicit `#pragma omp flush`
2. A `#pragma omp atomic` with an appropriate memory-ordering clause
3. The implicit barrier at the end of `parallel`, `for`, `single`, `sections`
4. `omp_set_lock` / `omp_unset_lock`
5. Task scheduling points (`taskwait`, end of `taskgroup`)

Between flush points, the compiler is free to keep values in registers or store buffers.

## `flush` — the explicit fence

```cpp
shared_data = 42;
#pragma omp flush(shared_data)   // make this specific variable visible
ready = true;
#pragma omp flush                // flush ALL currently accessible variables
```

A `flush` is a fence: writes before the flush become visible after it; reads after the flush see all writes-before. Without a list, all variables are flushed.

In practice, the implicit flush points at construct boundaries usually suffice. Explicit `flush` is needed mainly in lock-free patterns.

## The five atomic memory-ordering clauses (5.1)

Every `#pragma omp atomic` takes an optional ordering clause. `seq_cst` is the default and the safe choice; relaxed orderings are cheaper on weakly-ordered CPUs (arm64, POWER) but must be paired correctly.

| Clause | Legal on | Meaning |
|---|---|---|
| `seq_cst` | read / write / update / capture | Every `seq_cst` access sits in a single global total order. Strongest, default, safe. |
| `release` | write / update / capture | Prior reads and writes complete *before* this store. Pairs with a matching `acquire` load. |
| `acquire` | read / update / capture | Subsequent reads and writes happen *after* this load. Pairs with a matching `release` store. |
| `acq_rel` | update / capture | Acquire on the read part + release on the write part. Right for compare-and-swap and lock-free queues. |
| `relaxed` | any | Atomicity only — no ordering relative to other accesses. Use for standalone counters/statistics that don't gate any other state. |

### Acquire / release publish-subscribe idiom

```cpp
// Thread A (publisher):              // Thread B (subscriber):
data = 42;                            int seen = 0;
#pragma omp atomic write release      do {
ready = 1;                            #pragma omp atomic read acquire
                                        seen = ready;
                                      } while (!seen);
                                      printf("%d\n", data);   // sees 42
```

- The release-store synchronises-with the acquire-load: every memory effect *before* the release on A is observable *after* the acquire on B.
- Cheaper than `seq_cst` on weakly-ordered CPUs (arm64, POWER).

## Implicit flush summary

| Construct boundary | Implicit flush? |
|---|---|
| `parallel` entry / exit | Yes |
| `for` / `single` / `sections` exit (no `nowait`) | Yes |
| Explicit `barrier` | Yes |
| `atomic` (matching ordering) | Yes |
| `critical` entry / exit | Yes |
| `omp_set_lock` / `omp_unset_lock` | Yes |
| `taskwait`, `taskgroup` end | Yes |
| `for nowait` exit | **No** |
| `single nowait` exit | **No** |

For A2/A3, the implicit flushes at pragma boundaries almost always suffice.

## Related

- [[Data Races and TSan]] — what goes wrong without synchronisation.
- [[critical and atomic]] — the primitives that provide flush points.
- [[Barriers]] — phase synchronisation = implicit flush.
- [[Locks]] — `omp_set_lock` / `omp_unset_lock` also flush.
