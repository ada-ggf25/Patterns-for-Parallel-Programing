# Schedules

The `schedule` clause controls how iterations of a `parallel for` are distributed across threads. The right choice matters when per-iteration work is not uniform.

## Schedule kinds

| Schedule | Distribution | Best use |
|---|---|---|
| `static` | Contiguous equal-size chunks; one pass | Uniform cost per iteration |
| `static, C` | Chunks of size C, round-robin | Uniform cost, some NUMA control |
| `dynamic, C` | Threads pull C-iter chunks from a queue on demand | Irregular cost |
| `guided` | Chunks start large and shrink toward loop end | Cost decreasing toward loop end |
| `auto` | Implementation decides | Rarely useful — use explicit |
| `runtime` | Read `OMP_SCHEDULE` env var | Testing different schedules without recompile |

`C` is the chunk size — a tuning parameter.

## The spike workload problem

If some iterations cost 10× more than others, `static` clusters all the spikes on whichever thread owns that range. The fast threads finish and sit idle at the barrier:

```
Thread 0: ████ DONE ———————— wait at barrier
Thread 1: ████████████████ (has all the spikes)
Thread 2: ████ DONE ————————
Thread 3: ████ DONE ————————
           ↑ all idle until thread 1 finishes
```

`dynamic, C` or `guided` work-steal so expensive iterations distribute across the team.

## Code: three variants of the same kernel

```cpp
// static — one contiguous chunk per thread
#pragma omp parallel for default(none) shared(n) reduction(+:sum) schedule(static)
for (std::size_t i = 0; i < n; ++i) sum += busy_work(cost(i));

// dynamic, 64 — pull 64 iters on demand
#pragma omp parallel for default(none) shared(n) reduction(+:sum) schedule(dynamic, 64)
for (std::size_t i = 0; i < n; ++i) sum += busy_work(cost(i));

// guided — start big, shrink toward end
#pragma omp parallel for default(none) shared(n) reduction(+:sum) schedule(guided)
for (std::size_t i = 0; i < n; ++i) sum += busy_work(cost(i));
```

## Chunk size tuning (`dynamic, C`)

| C too small (= 1) | C too big (= N/P) |
|---|---|
| Queue dispatch overhead per chunk dominates | Same as `static` — imbalance returns |
| Cache lines bounce between threads | One thread gets a cluster of spikes |

### Picking C for a spike workload

For a **spike-every-10-iterations** workload (like A1's `f(x)`), a chunk of `C = 64` contains ~6–7 spike iterations. That's enough to average out the spike cost across the chunk — threads that pull an all-spike chunk and threads that pull an all-cheap chunk differ only modestly. Meanwhile `C = 64` amortises dispatch overhead: with N = 1e8 and 128 threads, a chunk of 64 means ~1.2 million dispatches instead of 1e8.

Rule of thumb: **choose C so that a typical chunk contains several spikes** (C ≈ 5–10 × spike-spacing) and is large enough that dispatch overhead < 5 % of the chunk's compute time. Measure — the crossover shifts with thread count and hardware.

For the A1 schedule sweep, start with `C ∈ {32, 64, 128}` and pick the timing winner at each thread count.

## Default schedule is implementation-defined

If no `schedule` clause appears, OpenMP picks for you — GCC and Clang use `static` with equal chunks. This is **not portable**. Always specify a schedule explicitly when iteration cost is non-uniform. A missing schedule is a correctness risk, not a performance risk, but it means your code's behaviour depends on the compiler version.

## Schedule-sweep methodology (A1)

For an unfamiliar workload, measure all three at each thread count `{1, 16, 64, 128}`:

```cpp
const double t_static  = time(sum_static, n);
const double t_dynamic = time(sum_dynamic_64, n);
const double t_guided  = time(sum_guided, n);
```

The winner can shift with thread count: at low counts, dispatch overhead of `dynamic` doesn't pay off; at high counts, load-balance wins. Record all three in `tables.csv`.

## Related

- [[for directive]] — the loop form and other clauses.
- [[for directive]] — `parallel for` canonical form, `nowait`, and `collapse`.
- [[../assessment/A1 Integration]] — A1 requires a schedule sweep.
