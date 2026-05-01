# Data Races and TSan

A data race is two threads accessing the same memory location concurrently, where at least one is writing and there is no synchronisation between them. The C++ standard says the program's behaviour is **undefined**.

## What races look like in practice

```cpp
int counter = 0;
#pragma omp parallel for
for (int i = 0; i < 100000; ++i) {
    ++counter;   // read-modify-write, NOT atomic
}
// Expected: 100000. Actual: anywhere from ~60000 to 100000 — varies per run.
```

Three common symptoms:

| Symptom | Cause |
|---|---|
| **Lost updates** | Two threads read the same old value, both increment, both write — one increment is lost. |
| **Torn reads** | One thread reads a partially-written multi-byte value. |
| **Reordered observations** | Thread B sees flag=1 before seeing the data the producer wrote. |

## Detecting races with ThreadSanitizer (TSan)

Compile with TSan:

```bash
clang++ -fopenmp -fsanitize=thread -g -O1 race.cpp -o race
./race
```

TSan intercepts every memory access at runtime and reports races:

```
WARNING: ThreadSanitizer: data race (pid=...)
  Write of size 4 at 0x... by thread T3:
    #0 main.omp_outlined race.cpp:19
  Previous write of size 4 at 0x... by main thread:
    #0 main.omp_outlined race.cpp:19
```

On the CI (GitHub Actions), TSan runs with the **Archer** OMPT tool (`libarcher.so`) which teaches TSan about OpenMP synchronisation boundaries — eliminating false positives from implicit barriers.

## The `default(none)` discipline

The root cause of most races in OpenMP code is variables accidentally defaulting to `shared`. Fix this by requiring every variable to be declared explicitly:

```cpp
// BAD — counter silently shared → race
#pragma omp parallel for
for (int i = 0; i < n; ++i) ++counter;

// GOOD — compiler error if any variable is not declared
#pragma omp parallel for default(none) shared(a, n) reduction(+:sum)
for (std::size_t i = 0; i < n; ++i) sum += a[i];
```

`default(none)` turns "accidental sharing" from a runtime mystery into a compile-time error. Use it in every parallel region.

## Fixing the race

The canonical fix for an accumulator is `reduction`:

```cpp
int counter = 0;
#pragma omp parallel for default(none) shared(n) reduction(+:counter)
for (int i = 0; i < n; ++i) ++counter;
// Now correct regardless of thread count or scheduling.
```

See [[reduction clause]] and [[User-Defined Reductions]] for when you need more than a simple scalar accumulator.

## Related

- [[Variable Scoping]] — shared / private / firstprivate rules.
- [[reduction clause]] — the canonical race fix for accumulators.
- [[critical and atomic]] — for cases where reduction can't be used.
- [[Memory Model]] — acquire/release semantics, flush, and visibility.
