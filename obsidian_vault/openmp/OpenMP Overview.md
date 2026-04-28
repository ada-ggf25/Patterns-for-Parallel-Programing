# OpenMP — Overview

OpenMP is a directive-based standard for **shared-memory** parallelism in C, C++, and Fortran. You add `#pragma omp ...` annotations to a serial program; the compiler emits multi-threaded code.

## Mental model

- All threads share one address space — they can read and write the same variables.
- Threads are created and destroyed by the runtime, not by you (`fork-join` — see [[Fork Join Model]]).
- The number of threads at runtime comes from `OMP_NUM_THREADS` (or, on PBS, the `ompthreads=N` selector).

## Pros

- Often the lowest-effort path from serial to "satisfactory speedup" — sometimes a single pragma is all it takes.
- Portable across compilers (GCC, Clang, Intel, NVHPC) and operating systems.
- Mature; broad library support.

## Cons

- **Confined to one node.** Shared memory means one machine. If your problem doesn't fit in one node's RAM, you need [[../mpi/MPI Overview|MPI]] instead.
- **Functional portability ≠ performance portability.** A pragma may run correctly on every compiler, but the speedup will vary wildly. NUMA, cache, and pinning all matter.
- **Implicit data sharing is dangerous.** Variables shared by default lead to [[Variable Scoping|race conditions]] if you forget to mark them private.

## When to reach for OpenMP

- The hot loop in your serial program is parallelisable.
- Your data already fits comfortably in one node's RAM.
- You want a quick first speedup before deciding whether to invest in MPI.

The course's running example, π by midpoint integration, is a textbook fit: every term in the sum is independent ([[../examples/Pi Algorithm]]), so a single `#pragma omp parallel for reduction(+:sum)` is the entire parallelisation.

## Course path

1. [[Fork Join Model]] — the execution model.
2. [[parallel directive]] — `#pragma omp parallel`.
3. [[for directive]] — work-sharing across iterations.
4. [[reduction clause]] — fixing the data race.
5. [[Variable Scoping]] — `shared` / `private` / `firstprivate` / `reduction`.
6. [[OMP Environment Variables]] — `OMP_NUM_THREADS` and friends.
7. [[Thread Pinning]] — `OMP_PROC_BIND`, `OMP_PLACES`.
8. [[Building OpenMP]] — `g++ -fopenmp`, CMake.
9. [[OpenMP PBS Script]] — putting it all together on CX3.
10. [[Amdahls Law]] — what speedup to expect.
11. [[OpenMP Pitfalls]] — common foot-guns.

## Related

- [[../mpi/OpenMP vs MPI]] — when to choose which.
- [[../examples/pi_openmp]] — the running example.
