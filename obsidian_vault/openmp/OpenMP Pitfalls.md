# OpenMP Pitfalls

The five most common ways an OpenMP program fails.

## 1. Forgot `-fopenmp` at build time

**Symptom:** Program runs fine but at single-thread speed; `threads=1` in the output.

**Cause:** The compiler treats `#pragma omp ...` as an unknown pragma and ignores it. The program is serial.

**Fix:** Add `-fopenmp` to the compile *and* link line. With CMake, `find_package(OpenMP REQUIRED)` + `target_link_libraries(... OpenMP::OpenMP_CXX)` does this for you. See [[Building OpenMP]].

## 2. Forgot `reduction(+:sum)`

**Symptom:** The answer changes between runs. Sometimes π = 3.14159...; sometimes π = 2.78... or 4.31...

**Cause:** Race condition on the accumulator. Multiple threads read-modify-write `sum` simultaneously and updates get lost. See [[reduction clause]].

**Fix:** `#pragma omp parallel for reduction(+:sum)`.

## 3. Loop index declared shared

**Symptom:** Compile error, or a runtime answer that's badly wrong.

**Cause:** If `i` is declared outside the `for` and not marked private, OpenMP may treat it as shared. Threads then fight over the loop counter.

**Fix:** Declare the counter inside the `for`: `for (long long i = 0; ...)`. The OpenMP standard makes the loop counter of `#pragma omp for` automatically private, but only when it's declared in the loop header. See [[Variable Scoping]].

## 4. Wrote `sum = f(i)` instead of `sum += f(i)`

**Symptom:** Result is just the *last* term, not the sum.

**Cause:** `sum = f(i)` overwrites instead of accumulates. `reduction(+:sum)` is happily reducing per-thread "last value seen" into one number.

**Fix:** Use `+=`. Then `reduction` does what you want.

## 5. Mismatched `ncpus` and `ompthreads` in PBS

**Symptom:** Asked for 16 cores; only 8 threads ran (wasted 8 cores). Or asked for 8 cores; 16 threads ran on top of each other (over-subscribed → slower than 8).

**Cause:** PBS's `ompthreads=` must match `ncpus=` for pure-OpenMP jobs.

**Fix:** Always set both equal:

```bash
#PBS -l select=1:ncpus=8:ompthreads=8:...
```

See [[../pbs/Resource Selection]] and [[OMP Environment Variables]].

## 6. Pragma typo silently compiles

**Symptom:** Program runs at single-thread speed; scaling curve is flat 1.0× everywhere.

**Cause:** `#pragma opm parallel` (or any misspelling) is treated as an unknown pragma and silently ignored. The code is serial — OpenMP never sees it. The answer is correct (no race), but speedup is exactly 1.0 regardless of thread count.

**Fix:** Check compiler output for `warning: unknown pragma ignored`. Clang emits this under `-Wall`; the course's `.clang-tidy` enforces it. The symptom is a perfectly flat speedup curve — suspiciously identical to the serial baseline time scaled by thread count.

```cpp
// BUG — typo: 'opm' not 'omp'
#pragma opm parallel for reduction(+:sum)   // silently ignored

// FIX
#pragma omp parallel for reduction(+:sum)   // correctly compiled
```

## 7. Using `critical` instead of `reduction` (correctness vs performance)

**Symptom:** The answer is correct, but the parallel version is slower than serial — sometimes *much* slower.

**Cause:** `critical` serialises every iteration through a global mutex. With `n = 1e8` iterations and 128 threads, the mutex is acquired 1e8 times — far more expensive than a scalar add.

```cpp
// CORRECT but serial-rate performance
double sum = 0.0;
#pragma omp parallel for default(none) shared(sum, a, n)
for (size_t i = 0; i < n; ++i) {
#pragma omp critical
    sum += a[i];          // one mutex acquire per iteration × n iterations
}

// FAST and correct
double sum = 0.0;
#pragma omp parallel for default(none) shared(a, n) reduction(+:sum)
for (size_t i = 0; i < n; ++i) {
    sum += a[i];          // private copy per thread; combined once at region exit
}
```

**Fix:** Always prefer `reduction` for accumulators. Only reach for `critical` when the update cannot be expressed as a reduction — e.g., appending to a `std::vector`.

## Bonus: tiny parallel regions

**Symptom:** Loop has `n=1000`; with 8 threads it's *slower* than serial.

**Cause:** Fork-join overhead is microseconds; if the loop body is sub-microsecond, parallelisation costs more than it gains.

**Fix:** Either parallelise an outer loop instead, or use `if (n > threshold)` clause to fall back to serial for small inputs.

## Related

- [[Variable Scoping]] — root cause of #2 and #3.
- [[reduction clause]] — fix for #2.
- [[Building OpenMP]] — fix for #1.
- [[../pbs/Common PBS Mistakes]] — overlapping checklist for the PBS side.
