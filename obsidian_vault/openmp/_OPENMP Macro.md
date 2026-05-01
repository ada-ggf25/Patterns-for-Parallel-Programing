# `_OPENMP` Macro and Version Detection

Every compliant OpenMP implementation `#define`s the `_OPENMP` macro to a `YYYYMM` integer. Use it for version assertions, portability shims, and serial fallbacks.

## Version values

| Macro value | Standard |
|---|---|
| `201511` | OpenMP 4.5 |
| `201811` | OpenMP 5.0 |
| **`202011`** | **OpenMP 5.1 — target for this course** |
| `202111` | OpenMP 5.2 |

## Compile-time guard

```cpp
#ifdef _OPENMP
    static_assert(_OPENMP >= 202011, "Course requires OpenMP 5.1+");
    std::printf("OpenMP version: %d\n", _OPENMP);
#else
    #error "OpenMP support required — compile with -fopenmp"
#endif
```

Put this at the top of your assessment files. The `static_assert` catches "wrong compiler module loaded"; the `#error` catches "compiled without `-fopenmp`".

## Runtime team queries

Outside a parallel region:
```cpp
omp_get_max_threads()   // max team size for the next region
```

Inside a parallel region:
```cpp
omp_get_num_threads()   // current team size
omp_get_thread_num()    // this thread's id in [0, team_size)
```

The "print once" idiom:
```cpp
#pragma omp parallel
{
    if (omp_get_thread_num() == 0)
        std::printf("team size: %d\n", omp_get_num_threads());
}
```

## Toolchain on CX3

```bash
ml tools/prod GCC CMake       # GCC with OpenMP 5.0 (fopenmp)
# For OpenMP 5.1 specifically:
ml LLVM                       # or use clang-18 from apt on Ubuntu/WSL
```

## Pragma typos silently compile

```cpp
#pragma opm parallel     // typo → compiler ignores, code runs serial
```

Compilers treat unknown pragmas as no-ops. No error, no warning — just serial execution. Symptoms: mysteriously flat speedup curve. Check build output for `unrecognised #pragma` — Clang emits this under `-Wall`.

## Related

- [[Building OpenMP]] — how to compile with `-fopenmp`.
- [[OMP Environment Variables]] — `OMP_NUM_THREADS`, `OMP_PROC_BIND`, etc.
- [[parallel directive]] — the region this enables.
