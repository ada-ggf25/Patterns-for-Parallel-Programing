# The Fork-Join Model

OpenMP's execution model is **fork-join**: the program starts and ends with one thread (the *master*), and parallel regions create teams of threads that join back to the master at the end.

```
  [master thread, running serially]
       |
       v
   #pragma omp parallel        <-- fork N threads
   {
       |---+---+---+---+
       |   |   |   |   |
       |   |   |   |   |       <-- each thread runs the body
       |   |   |   |   |
       |---+---+---+---+
   }                            <-- join: only master continues
       |
       v
  [master continues serially]
```

## Key implications

- **Serial code stays serial.** Anything outside a `#pragma omp parallel` region runs on one thread (the master). Forgetting to put work inside a parallel region is a common source of "OpenMP didn't speed anything up" bug reports.
- **Each parallel region pays a fork-join cost.** Spinning up threads, distributing work, and joining at the end takes microseconds. For very short loops, the overhead dwarfs the gain — keep parallel regions chunky.
- **Threads are reused across regions.** The OpenMP runtime keeps a thread pool; the "fork" is logical, not a fresh `pthread_create` each time. So the cost in steady state is small.
- **Master joins, then continues alone.** State written by worker threads inside the region is visible after the region ends, but if you wanted to keep multi-threaded execution across two regions back-to-back you don't need to "re-fork" — just put both inside one parallel region.

## Two ways to fork

```cpp
#pragma omp parallel              // fork; team active until end of block
{
    #pragma omp for               // work-share inside the team
    for (int i = 0; i < n; ++i) ...
}

// vs. the combined form:
#pragma omp parallel for          // fork + work-share in one go
for (int i = 0; i < n; ++i) ...
```

The combined form is the common shortcut. The split form is useful when you have multiple work-sharing constructs to amortise the same fork over.

## Related

- [[parallel directive]] — the fork.
- [[for directive]] — the work-sharing.
- [[OpenMP Overview]] — bigger context.
