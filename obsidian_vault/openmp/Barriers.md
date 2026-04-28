# Barriers

A barrier is a synchronisation point where every thread must arrive before any thread proceeds. Barriers enforce phase separation: all "produce" work completes before any "consume" work starts.

## Implicit barriers

OpenMP inserts an implicit barrier at the end of these constructs:

- `parallel` region exit
- `for` worksharing loop exit
- `single` block exit
- `sections` exit
- Explicit `barrier` directive

The implicit barrier at the end of a `for` is what makes two back-to-back loops correct:

```cpp
#pragma omp parallel default(none) shared(a, b, n)
{
#pragma omp for                    // implicit barrier here
    for (size_t i = 0; i < n; ++i) a[i] = init(i);

#pragma omp for
    for (size_t i = 0; i < n; ++i) b[i] = a[i] * 2;  // safe: all a[i] are written
}
```

## Explicit barrier

```cpp
#pragma omp parallel
{
    do_phase_a();
#pragma omp barrier                // wait for all threads
    do_phase_b();
}
```

Use an explicit barrier when you need a phase boundary inside a `parallel` region without a worksharing construct.

## `nowait` — lifting the implicit barrier

```cpp
#pragma omp parallel default(none) shared(a, b, n)
{
#pragma omp for nowait             // no barrier — fast threads continue
    for (size_t i = 0; i < n; ++i) a[i] = 1.0;

#pragma omp for
    for (size_t j = 0; j < n; ++j) b[j] = 2.0;  // unrelated to a[i]
}
```

`nowait` is safe **only when the next stage has no dependence on the previous one**. If the second loop read `a[i]`, this would be a race. When in doubt, leave the barrier in.

## The orphan-construct rule

A worksharing directive can appear *inside a function* and bind to the enclosing `parallel` region at the call site:

```cpp
void worker(std::vector<double>& v) {
#pragma omp for default(none) shared(v)   // orphaned for
    for (size_t i = 0; i < v.size(); ++i) v[i] *= 2.0;
}

#pragma omp parallel
{ worker(v); }   // orphan binds here
```

Outside any `parallel`, `worker` runs serially — no error, just no parallelism.

## Related

- [[single and masked]] — `single nowait` removes the barrier from a single block.
- [[critical and atomic]] — other synchronisation primitives.
- [[Tasks]] — `taskwait` / `taskgroup` for task-based synchronisation.
