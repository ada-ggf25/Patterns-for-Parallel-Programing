# `critical` and `atomic`

Two synchronisation primitives for protecting shared updates. Use them when `reduction` can't express what you need.

## `critical` — general-purpose mutex

```cpp
#pragma omp critical
{
    // Only one thread inside at a time — arbitrary code allowed.
    update_shared_data_structure();
}
```

- Anonymous critical uses one global mutex. Very expensive at high thread counts.
- Named critical: two regions with different names are independent mutexes:

```cpp
#pragma omp critical (bucket_a) { update_a(); }
#pragma omp critical (bucket_b) { update_b(); }  // runs in parallel with bucket_a
```

Granularity is tied to source code, not data — you can't have one mutex per array element. For data-driven granularity, use [[Locks]].

## `atomic` — hardware-level single-op

```cpp
#pragma omp atomic
counter += 1;   // compiled to lock xadd (x86) or LLDSC (arm64) — no OS lock
```

`atomic` is restricted to a single supported read-modify-write expression:
- `x op= expr` — `+=`, `-=`, `*=`, `/=`, `&=`, `|=`, `^=`
- `x++`, `++x`, `x--`, `--x`
- `x = x op expr`

Anything more complex needs `critical`.

## Four atomic clauses (OpenMP 5.1)

```cpp
#pragma omp atomic read
v = shared_x;

#pragma omp atomic write
shared_x = v;

#pragma omp atomic update     // default if no clause given
shared_x += delta;

#pragma omp atomic capture    // read + update atomically
{ tmp = shared_x; shared_x += 1; }
```

## Memory-ordering clause (5.1)

```cpp
#pragma omp atomic write release    // pairs with an acquire load elsewhere
ready = 1;
```

Available orderings: `seq_cst` (default — strongest), `acquire`, `release`, `acq_rel`, `relaxed`. See [[Memory Model]] for when to use each.

## Choosing the right tool

| Need | Use |
|---|---|
| Accumulate many independent updates | `reduction` |
| Single arithmetic RMW on a scalar | `atomic update` |
| Small block under one global mutex | `critical` |
| Many independent locks, one per data instance | `omp_lock_t` (see [[Locks]]) |
| Complex struct update under mutex | `critical` (or finer locks) |

**Cost ordering at high thread counts:** `critical` ≫ `atomic` ≫ `reduction`.

Default to `reduction`; reach for `atomic` only when you can't accumulate, and `critical` only when neither works.

## The critical-instead-of-reduction trap

```cpp
// BAD — serialises every iteration; slower than serial
double sum = 0.0;
#pragma omp parallel for default(none) shared(sum, a, n)
for (size_t i = 0; i < n; ++i) {
#pragma omp critical
    sum += a[i];    // correct but ~P× slower than serial at high P
}

// GOOD
#pragma omp parallel for default(none) shared(a, n) reduction(+:sum)
for (size_t i = 0; i < n; ++i) sum += a[i];
```

## Related

- [[reduction clause]] — should almost always be your first choice.
- [[Locks]] — data-driven granularity.
- [[Memory Model]] — acquire/release semantics.
- [[Barriers]] — phase synchronisation (not mutual exclusion).
