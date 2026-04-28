# `reduction(op:var)`

The fix for accumulators in a parallel loop. Without it, multiple threads racing to read-modify-write the same accumulator produce nondeterministic, almost-always-wrong results.

## The race condition

```cpp
double sum = 0.0;
#pragma omp parallel for          // <-- BUG: race on `sum`
for (long long i = 1; i <= n; ++i) {
    sum += f(i);
}
```

Every thread reads `sum`, computes `sum + f(i)`, and writes back. Two threads doing this at once may both read the same old value, both add their own contribution, and both write — losing one of them. The answer depends on thread timing: the program is non-deterministic.

## The fix

```cpp
double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
for (long long i = 1; i <= n; ++i) {
    sum += f(i);
}
```

What the runtime does:

1. Gives each thread its own private `sum` variable, initialised to the identity for the operator (0 for `+`, 1 for `*`, etc.).
2. Each thread accumulates into its private copy — no contention.
3. At the end of the loop, all private values are combined into the master's `sum` using the operator.

The result is deterministic *up to floating-point rounding* — `+` on doubles is not associative, so different thread counts can produce slightly different last-bit results. For π via midpoint rule with N=1e9, you'll see the result agree to ~14 decimal places.

## Supported operators

| Operator | Identity |
|---|---|
| `+` | 0 |
| `*` | 1 |
| `-` | 0 |
| `&` | ~0 (all bits) |
| `|` | 0 |
| `^` | 0 |
| `&&` | 1 |
| `\|\|` | 0 |
| `min` | largest representable |
| `max` | smallest representable |

OpenMP 4.0+ also supports user-defined reductions with `#pragma omp declare reduction`, useful for combining custom accumulator structs.

## In the running example

The OpenMP version of π differs from the serial version by exactly **one line**:

```cpp
#pragma omp parallel for reduction(+:sum)         // <-- only change
for (long long i = 1; i <= n; ++i) {
    const double x = w * (static_cast<double>(i) - 0.5);
    sum += 4.0 / (1.0 + x * x);
}
```

That's it — see [[../examples/pi_openmp]] for the full file.

## Related

- [[for directive]] — the loop being parallelised.
- [[Variable Scoping]] — broader picture of shared/private variables.
- [[OpenMP Pitfalls]] — what goes wrong without `reduction`.
