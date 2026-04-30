# Loop Transformations

## `collapse(N)` — fuse nested loops

Without `collapse`, `parallel for` distributes only the outermost loop. If the outer loop has too few iterations to fill the team, most threads sit idle:

```cpp
// PROBLEM: only 4 outer iterations — most of 128 threads idle
#pragma omp parallel for
for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 1000; ++j)
        work(i, j);
```

`collapse(2)` fuses the two loops into a single iteration space of `4 × 1000 = 4000` iterations, distributed across all threads:

```cpp
#pragma omp parallel for collapse(2)
for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 1000; ++j)
        work(i, j);
```

## Requirements for `collapse`

Loops must be *perfectly nested* canonical loops:

- No code between the outer loop body and the inner loop.
- Inner trip count independent of outer index.
- Both loops use canonical form (no `break`, no early return).

```cpp
// OK — perfectly nested, independent bounds
#pragma omp parallel for collapse(2)
for (int i = 0; i < N; ++i)
    for (int j = 0; j < M; ++j) ...

// NOT OK — code between loops; can't collapse
for (int i = 0; i < N; ++i) {
    int tmp = setup(i);            // ← breaks perfect nesting
    for (int j = 0; j < M; ++j) ...
}
```

## `collapse(3)` for A3 Jacobi

The 3D Jacobi stencil has three nested loops. With `collapse(3)` all (NX-2)×(NY-2)×(NZ-2) interior iterations are distributed (≈ 133 M for 512³):

```cpp
#pragma omp parallel for collapse(3) default(none) shared(u, u_next)
for (size_t i = 1; i < NX-1; ++i)
    for (size_t j = 1; j < NY-1; ++j)
        for (size_t k = 1; k < NZ-1; ++k)
            u_next[idx(i,j,k)] = stencil(u, i, j, k);
```

`NX`, `NY`, `NZ` are `constexpr` constants — they are not runtime variables and do **not** need to appear in `shared` or `firstprivate`. This gives all 128 threads enough work even when one dimension is small.

## Tiling (preview — A3 advanced)

For memory-bound kernels, *tiling* improves cache reuse by processing small blocks that fit in cache:

```cpp
// Without tiling: streams the entire grid through cache at every step
for (size_t step = 0; step < NSTEPS; ++step)
    for (size_t i = ...) ...

// With tiling: reuse small blocks across multiple timesteps
for (size_t step = 0; step < NSTEPS; ++step)
    for (size_t ii = 0; ii < NX; ii += TILE)
        for (size_t jj = 0; jj < NY; jj += TILE)
            for (size_t i = ii; i < ii + TILE; ++i)
                for (size_t j = jj; j < jj + TILE; ++j) ...
```

Not required for A3-core; valuable for the A3 extension if you want to push the roofline fraction higher.

## Related

- [[SIMD]] — vectorise the inner loop after collapsing the outer ones.
- [[Roofline Model]] — tiling improves effective OI.
- [[../openmp/for directive]] — `collapse` is a clause on `for`.
- [[../assessment/A3 Jacobi]] — collapse is the primary loop transformation for A3.
