# User-Defined Reductions

Built-in `reduction(+:x)` works on scalars. When you need to reduce a **compound struct** (e.g. accumulating count + sum + sum-of-squares in one pass), use `declare reduction`.

## Syntax

```cpp
#pragma omp declare reduction(
    <name> : <type> : <combiner>
) initializer(<init>)
```

- `<name>` — operator name you invent (e.g. `stats_add`).
- `<type>` — the C++ type being reduced.
- `<combiner>` — expression combining `omp_out` (running accumulator) and `omp_in` (incoming partial). Must be associative and commutative.
- `initializer` — expression that sets a new thread's private copy to the identity.

## Example: mean and variance in one pass

```cpp
struct Stats {
    long long n   = 0;
    double    sum = 0.0;
    double    sq  = 0.0;
};

#pragma omp declare reduction(stats_add : Stats :
    omp_out.n   += omp_in.n;
    omp_out.sum += omp_in.sum;
    omp_out.sq  += omp_in.sq
) initializer(omp_priv = Stats{})

Stats s;
#pragma omp parallel for default(none) shared(a, n) reduction(stats_add : s)
for (std::size_t i = 0; i < n; ++i) {
    ++s.n;
    s.sum += a[i];
    s.sq  += a[i] * a[i];
}

const double mean = s.sum / s.n;
const double var  = s.sq  / s.n - mean * mean;
```

One parallel pass yields both mean and variance.

## Magic variable names

| Name | Meaning |
|---|---|
| `omp_out` | The running accumulator (thread's own partial result) |
| `omp_in` | The incoming partial result being merged |
| `omp_priv` | In `initializer`, the new private copy being initialised |

## When to use UDR

Use `declare reduction` whenever you find yourself wrapping a compound struct update inside `#pragma omp critical` — that's almost always a sign a user-defined reduction is the right tool:

```cpp
// BAD — serialises everything
#pragma omp critical
{ s.sum += a[i]; s.n += 1; }

// GOOD — parallel private copies, combined at exit
reduction(stats_add : s)
```

## Related

- [[reduction clause]] — built-in scalar reductions.
- [[Data Races and TSan]] — why you can't just share the struct.
- [[critical and atomic]] — when reduction can't express what you need.
