# SIMD with OpenMP

SIMD (Single Instruction, Multiple Data) executes one instruction on a vector of values simultaneously. Rome's Zen 2 cores have 256-bit AVX2 registers: 4 doubles or 8 floats per register, per instruction.

## AVX2 basics on Rome

- 256-bit registers → **4 doubles** per SIMD instruction.
- One FMA (`vfmadd`) per cycle = 2 FLOPs × 4 lanes = 8 FLOPs/cycle.
- Per-core peak: 2.25 GHz × 16 FLOPs/cycle = 36 GFLOPs.
- Node-wide: 128 cores × 36 = **4608 GFLOPs** (theoretical).

## `omp simd` — vectorise within one thread

```cpp
#pragma omp simd
for (size_t i = 0; i < n; ++i)
    r[i] = a * x[i] + y[i];   // AXPY — each iter independent
```

`simd` tells the compiler: "this loop has no cross-iteration dependences — vectorise it." Even without the pragma, auto-vectorisation often fires, but `simd` makes the intent explicit and asserts correctness.

## `parallel for simd` — threading + vectorisation

```cpp
#pragma omp parallel for simd default(none) shared(a, x, y, r, n)
for (size_t i = 0; i < n; ++i)
    r[i] = a * x[i] + y[i];
```

Two levels of parallelism: threads distribute iterations, then each thread vectorises its chunk. Compose both for free when the loop is amenable.

## Aligned allocation for SIMD

```cpp
void* raw = nullptr;
posix_memalign(&raw, 64, n * sizeof(double));
double* x = static_cast<double*>(raw);
// ... use x in #pragma omp simd loops ...
std::free(x);
```

- **What alignment to ask for?** `64` bytes is the universal answer:
  - AVX2 vector width is 32 bytes; aligning to 32 covers AVX/AVX2.
  - Cache line is 64 bytes; aligning to 64 covers all SIMD widths up to AVX-512 *and* aligns to cache-line boundaries — which doubles as the false-sharing-prevention size. One number to remember.
- **Why not `std::vector<double>(n)`?** Two problems: the constructor value-initialises on the master (defeating NUMA first-touch), *and* gives no alignment guarantee beyond `alignof(double) = 8`. Wrong for both reasons.
- **Why not the `aligned(p : 64)` clause on `#pragma omp simd`?** On Zen 2 unaligned vector loads cost ~the same as aligned, so the compiler-side win is ~0 % here. Save it for ARM / older Intel where it pays off.
- The same `posix_memalign` you use for first-touch is the right tool for SIMD-friendly memory — same alignment constant, same function.

## `aligned` and `safelen` (advanced, optional)

```cpp
#pragma omp simd aligned(x, y, r : 64) safelen(8)
for (size_t i = 0; i < n; ++i)
    r[i] = a * x[i] + y[i];
```

- `aligned(x, y, r : 64)` — promise that pointers are 64-byte aligned; compiler emits aligned load/store instructions. **Misaligning at runtime → segfault.**
- `safelen(8)` — assert at least 8 consecutive iterations are dependence-free.

**Current guidance**: on Zen 2 (Rome) and any post-Haswell Intel, unaligned vector loads/stores cost essentially the same as aligned ones. Alignment tweaks are therefore out of scope for A3. Plain `#pragma omp simd` is sufficient and correct.

## `declare simd` — vectorised function variant

```cpp
// Annotate the function to emit a SIMD variant alongside the scalar one:
#pragma omp declare simd notinbranch
double smooth_step(double t) {
    return t * t * (3.0 - 2.0 * t);
}

// Consumer loop — compiler picks SIMD variant automatically:
#pragma omp simd
for (size_t i = 0; i < n; ++i)
    out[i] = smooth_step(in[i]);
```

Without `declare simd`, the compiler either inlines the body (if it can see it) or makes per-element scalar calls — defeating vectorisation.

## Reading vectorisation reports

```bash
clang++ -O3 -fopenmp-version=51 \
  -Rpass=loop-vectorize \
  -Rpass-missed=loop-vectorize \
  source.cpp
```

```
source.cpp:24:5: remark: vectorized loop (width: 4) [-Rpass=loop-vectorize]
source.cpp:38:5: remark: loop not vectorized: cannot identify array bounds
                 [-Rpass-missed=loop-vectorize]
```

`-Rpass-missed` is the actionable one: add `#pragma omp simd`, fix array-bounds aliasing, change pointer types, or add `restrict` to eliminate alias analysis failure.

## A3 extension: SIMD

A3's SIMD extension: annotate the Jacobi innermost loop with `#pragma omp simd`; measure the speedup ratio. Soft threshold: `ratio ≥ 1.2×` → full marks; `≥ 1.05×` → half marks.

```cpp
// before — relies on auto-vectorisation
for (size_t i = ...) r[i] = stencil(u, i);

// after
#pragma omp simd
for (size_t i = ...) r[i] = stencil(u, i);
```

## Related

- [[Loop Transformations]] — `collapse` for distributing multi-dimensional loops.
- [[Roofline Model]] — SIMD moves toward the compute ceiling.
- [[../assessment/A3 Jacobi]] — A3-extension/simd.
